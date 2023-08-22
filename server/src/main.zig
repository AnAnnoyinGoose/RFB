const std = @import("std");
const allocator = std.heap.page_allocator;

const server = @import("server.zig").Server;

pub fn main() !void {
    var s = try server.init(allocator, 344);
    defer s.deinit();
    try s.run();
}
