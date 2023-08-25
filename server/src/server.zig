const std = @import("std");
const file = @import("file_browser.zig");

const VERSION = "1.0.0";

const Reply = enum { ok, err, unknown };

pub const Server = struct {
    port: u16,
    allocator: std.mem.Allocator,
    server: std.net.StreamServer,

    pub fn init(allocator: std.mem.Allocator, port: u16) !Server {
        const addr = try std.net.Address.parseIp4("0.0.0.0", port);
        var server = std.net.StreamServer.init(.{
            .reuse_address = true,
        });
        try server.listen(addr);
        return Server{
            .port = port,
            .allocator = allocator,
            .server = server,
        };
    }
    pub fn deinit(self: *Server) void {
        self.server.deinit();
    }
    pub fn run(self: *Server) !void {
        while (true) {
            const conn = try self.server.accept();
            defer conn.stream.close();
            try self.handle(conn);
            std.debug.print("Connection closed\n", .{});
        }
    }

    fn acceptor(self: *Server, buf: []u8) Reply {
        _ = self;
        if (buf.len < 8) {
            return .err;
        }
        if (std.mem.eql(u8, buf, "RFBv0.1nR")) {
            return .ok;
        }
        return .err;
    }
    fn send_basic_data(self: *Server, conn: std.net.StreamServer.Connection) !void {
        const arr = file.fetch_files_from_folder("public") catch |err| {
            std.debug.print("Array Error: {s}\n", .{@errorName(err)});
            return;
        };
        var data_all = std.ArrayList([]const u8).init(self.allocator);
        defer data_all.deinit();
        for (arr) |f| {
            // concat the path with a newline
            const data = try std.fmt.allocPrint(self.allocator, "{s} \n\r {d} \n\r {s}\n", .{ f.path, f.size, f.mime });
            data_all.append(data) catch |err| {
                std.debug.print("Array Error: {s}\n", .{@errorName(err)});
                return;
            };
        }
        _ = try conn.stream.write("SfL\n"); // start of file list
        std.time.sleep(100);
        for (data_all.items) |data| {
            _ = try conn.stream.write(data);
            std.time.sleep(100);
        }
        _ = try conn.stream.write("EfL\n");
    }

    fn loginator(self: *Server, conn: std.net.StreamServer.Connection) !Reply {
        _ = self;
        const password = "password\n";
        const RFB_L = "RFB-L\n";
        var buf: [1024]u8 = undefined;
        var size = try conn.stream.read(&buf);
        if (std.mem.eql(u8, buf[0..5], RFB_L)) {
            std.debug.print("memeql Input: {s} Expected: {s}\n", .{ buf[0..5], RFB_L });
            return .err;
        }
        _ = try conn.stream.write("RFB-L-IN");
        size = try conn.stream.read(&buf);
        if (std.mem.eql(u8, buf[0..size], password)) {
            _ = try conn.stream.write("RFB-L-OK\n");
            return .ok;
        }
        return .err;
    }

    fn handle(self: *Server, conn: std.net.StreamServer.Connection) !void {
        var buf: [1024]u8 = undefined;
        const size = try conn.stream.read(&buf);
        switch (self.acceptor(buf[0..size])) {
            .ok => {
                _ = try conn.stream.write("RFBv0.1nR\n");
                switch (try self.loginator(conn)) {
                    .ok => {
                        try self.send_basic_data(conn);
                    },
                    .err => {
                        _ = try conn.stream.write("Wrong password\n");
                    },
                    .unknown => {},
                }
            },
            .err => {
                _ = try conn.stream.write("Sum ting wong\n");
            },
            .unknown => {},
        }
    }
};
