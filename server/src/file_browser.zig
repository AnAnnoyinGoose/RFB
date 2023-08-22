const std = @import("std");
const os = std.os;

const Ownership_Enum = enum { public, private };
const FileType = enum { IMAGE, TEXT, EXECUTABLE, VIDEO, AUDIO, DIRECTORY, UNKNOWN };

const Ownership = union(Ownership_Enum) {
    public: void,
    private: []const u8, // name of the owner
};

const File = struct {
    name: []const u8,
    path: []const u8,
    data: []const u8,
    size: usize,
    md5: [16]u8,
    mime: []const u8,
    permissions: os.mode_t,
    date: ?i128,
    last_modified: ?i128,
    ownership: Ownership,
    type: FileType,

    pub fn init(
        name: []const u8,
        path: []const u8,
        content: []const u8,
        size: usize,
        md5: [16]u8,
        mime: []const u8,
        permissions: os.mode_t,
        date: ?i128,
        last_modified: ?i128,
        ownership: Ownership,
        file_type: FileType,
    ) File {
        return File{
            .name = name,
            .path = path,
            .data = content,
            .size = size,
            .md5 = md5,
            .mime = mime,
            .permissions = permissions,
            .date = date,
            .last_modified = last_modified,
            .ownership = ownership,
            .type = file_type,
        };
    }

    pub fn print(self: File) !void {
        const cdate = self.date orelse 0;
        const last_modified = self.last_modified orelse 0;

        // Print the entire struct variable in one line
        std.io.getStdOut().writer().print(
            \\name: {s}
            \\path: {s}
            \\data: {s}
            \\size: {d}
            \\md5: {s}
            \\mime: {s}
            \\permissions: {d}
            \\date: {d}
            \\last_modified: {d}
            \\ownership: {s}
            \\type: {s}
            \\
        , .{
            self.name,
            self.path,
            self.data,
            self.size,
            self.md5,
            self.mime,
            self.permissions,
            cdate,
            last_modified,
            @tagName(self.ownership),
            @tagName(self.type),
        }) catch {};
    }
};
fn getMime(extension: []const u8) ![]const u8 {
    // a map of mime types to extensions
    comptime var mime_map = std.ComptimeStringMap([]const u8, .{
        // Text based
        .{ ".txt", "text/plain;charset=utf-8" },
        .{ ".md", "text/markdown" },
        .{ ".html", "text/html" },
        .{ ".css", "text/css" },
        .{ ".json", "application/json" },
        .{ ".xml", "application/xml" },
        .{ ".csv", "text/csv" },
        .{ ".tsv", "text/tab-separated-values" },
        .{ ".rtf", "application/rtf" },
        .{ ".pdf", "application/pdf" },
        .{ ".epub", "application/epub+zip" },
        .{ ".zip", "application/zip" },
        .{ ".gz", "application/gzip" },
        .{ ".tar", "application/x-tar" },
        .{ ".rar", "application/x-rar-compressed" },
        .{ ".7z", "application/x-7z-compressed" },
        .{ ".exe", "application/x-msdownload" },
        .{ ".dll", "application/x-msdownload" },
        .{ ".msi", "application/x-msdownload" },
        .{ ".doc", "application/msword" },
        .{ ".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
        .{ ".ppt", "application/vnd.ms-powerpoint" },
        .{ ".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
        .{ ".xls", "application/vnd.ms-excel" },
        .{ ".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
        .{ ".odt", "application/vnd.oasis.opendocument.text" },
        .{ ".ods", "application/vnd.oasis.opendocument.spreadsheet" },
        .{ ".odg", "application/vnd.oasis.opendocument.graphics" },
        .{ ".odp", "application/vnd.oasis.opendocument.presentation" },
        // Binary
        .{ ".bin", "application/octet-stream" },
        // Images
        .{ ".jpg", "image/jpeg" },
        .{ ".jpeg", "image/jpeg" },
        .{ ".png", "image/png" },
        .{ ".gif", "image/gif" },
        .{ ".bmp", "image/bmp" },
        .{ ".ico", "image/vnd.microsoft.icon" },
        .{ ".tiff", "image/tiff" },
        .{ ".tif", "image/tiff" },
        .{ ".webp", "image/webp" },
        // Audio
        .{ ".mp3", "audio/mpeg" },
        .{ ".wav", "audio/wav" },
        .{ ".ogg", "audio/ogg" },
        .{ ".flac", "audio/flac" },
        // Video
        .{ ".mp4", "video/mp4" },
        .{ ".webm", "video/webm" },
        .{ ".mkv", "video/x-matroska" },
        .{ ".avi", "video/x-msvideo" },
        .{ ".mov", "video/quicktime" },
        // Programming languages
        .{ ".py", "text/x-python" },
        .{ ".go", "text/x-go" },
        .{ ".js", "text/javascript" },
        .{ ".ts", "text/typescript" },
        .{ ".rs", "text/rust" },
        .{ ".java", "text/x-java" },
        .{ ".c", "text/x-c" },
        .{ ".cpp", "text/x-c++" },
        .{ ".h", "text/x-c" },
        .{ ".cs", "text/x-csharp" },
        .{ ".kt", "text/x-kotlin" },
        .{ ".rb", "text/x-ruby" },
        .{ ".lua", "text/x-lua" },
        .{ ".sh", "text/x-shellscript" },
        .{ ".swift", "text/x-swift" },
        .{ ".zig", "text/x-zig" },
    });
    return mime_map.get(extension) orelse return "application/octet-stream";
}

pub fn files_to_string(data: []File) ![]const u8 {
    var delim = "\n\r\n";
    var buf = std.ArrayList(u8).init(std.heap.page_allocator);
    defer buf.deinit();
    for (data) |file| {
        buf.writer().print(
            \\name: {s} 
            \\path: {s} 
            \\data: {s} 
            \\size: {d} 
            \\md5: {s}  
            \\mime: {s} 
            \\permissions: {d} 
            \\date: {d} 
            \\last_modified: {d} 
            \\ownership: {s} 
            \\type: {s}  
            \\
        , .{
            file.name,
            file.path,
            file.data,
            file.size,
            file.md5,
            file.mime,
            file.permissions,
            file.date orelse 0,
            file.last_modified orelse 0,
            @tagName(file.ownership),
            @tagName(file.type),
        }) catch {};
        buf.writer().writeAll(delim) catch {};
    }
    return buf.toOwnedSlice();
}

pub fn fetch_files_from_folder(dir: []const u8) ![]File {
    const alloc = std.heap.page_allocator;
    var files = std.ArrayList(File).init(alloc);
    defer files.deinit();

    var dir_it = try std.fs.cwd().openIterableDir(dir, .{});
    defer dir_it.close();
    var it = dir_it.iterate();

    while (try it.next()) |entry| {
        if (entry.kind == std.fs.File.Kind.Directory) {
            const new_dir = try std.fs.path.joinZ(alloc, &[_][]const u8{ dir, entry.name });
            std.debug.print("Directory: {s}\n", .{new_dir});
            const new_files = fetch_files_from_folder(new_dir) catch |err| {
                std.debug.print("Directory read Error: {s}\n", .{@errorName(err)});
                return files.toOwnedSlice();
            };
            files.appendSlice(new_files) catch |err| {
                std.debug.print("Error: {s}\n", .{@errorName(err)});
                return files.toOwnedSlice();
            };
        } else if (entry.kind == std.fs.File.Kind.File) {
            const path = try std.fs.path.joinZ(alloc, &[_][]const u8{ dir, entry.name });
            const data = try std.fs.cwd().openFile(path, .{});
            const metadata = try data.metadata();
            std.debug.print("File: {s}\n", .{path});
            var content = std.fs.cwd().readFileAlloc(alloc, path, std.math.maxInt(usize)) catch |err| {
                std.debug.print("File read Error: {s}\n", .{@errorName(err)});
                return files.toOwnedSlice();
            };
            var hash: [16]u8 = undefined;
            const mime = try getMime(std.fs.path.extension(entry.name));
            std.crypto.hash.Md5.hash(content, &hash, std.crypto.hash.Md5.Options{});

            const cdate = metadata.created();
            const ldate = metadata.modified();

            const file = File.init(
                entry.name,
                path,
                content,
                content.len,
                hash,
                mime,
                metadata.permissions().inner.mode,
                cdate,
                ldate,
                Ownership.public,
                FileType.UNKNOWN,
            );
            try files.append(file);
        }
    }
    return files.toOwnedSlice();
}
