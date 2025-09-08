const std = @import("std");
const spsc_queue = @import("spsc_queue");
const spsc_ring = @import("spsc_ring");

const total_rounds: u64 = 10_000_000;
const capacity: usize = 16_777_216; // This is power of two which is needed for cdolan implementation

fn spscReadWorker(q: *spsc_queue.SpscQueue(i32, false), rounds: u64) void {
    var i: u64 = 0;
    while (i < rounds) {
        while (q.front() == null) {}
        const val = q.front().?.*;
        if (val != i) @panic("out of order");
        q.pop();
        i += 1;
    }
}

fn spscPo2ReadWorker(q: *spsc_queue.SpscQueue(i32, true), rounds: u64) void {
    var i: u64 = 0;
    while (i < rounds) {
        while (q.front() == null) {}
        const val = q.front().?.*;
        if (val != i) @panic("out of order");
        q.pop();
        i += 1;
    }
}

fn spscRingReadWorker(r: *spsc_ring.Ring(i32), rounds: u64) void {
    var i: u64 = 0;
    while (i < rounds) {
        var got = r.dequeue();
        while (got == null) {
            got = r.dequeue();
        }
        const val: i32 = got.?;
        if (@as(u64, @intCast(val)) != i) @panic("out of order (ring)");
        i += 1;
    }
}

pub fn main() !void {
    {
        var queue = try spsc_queue.SpscQueue(i32, true).initCapacity(std.heap.page_allocator, capacity);
        defer queue.deinit();

        var reader = try std.Thread.spawn(.{}, spscPo2ReadWorker, .{ &queue, total_rounds });

        const start_ns_q: i128 = std.time.nanoTimestamp();

        var i_q: i32 = 0;
        while (i_q < total_rounds) : (i_q += 1) {
            queue.push(i_q);
        }

        reader.join();

        const end_ns_q: i128 = std.time.nanoTimestamp();
        const elapsed_ns_q: u128 = @intCast(end_ns_q - start_ns_q);
        const ops_per_ms_q: u128 = (@as(u128, total_rounds) * 1_000_000) / elapsed_ns_q;

        std.debug.print("freref-po2:\n {d} ops/ms\n", .{ops_per_ms_q});
    }

    {
        var queue = try spsc_queue.SpscQueue(i32, false).initCapacity(std.heap.page_allocator, capacity);
        defer queue.deinit();

        var reader = try std.Thread.spawn(.{}, spscReadWorker, .{ &queue, total_rounds });

        const start_ns_q: i128 = std.time.nanoTimestamp();

        var i_q: i32 = 0;
        while (i_q < total_rounds) : (i_q += 1) {
            queue.push(i_q);
        }

        reader.join();

        const end_ns_q: i128 = std.time.nanoTimestamp();
        const elapsed_ns_q: u128 = @intCast(end_ns_q - start_ns_q);
        const ops_per_ms_q: u128 = (@as(u128, total_rounds) * 1_000_000) / elapsed_ns_q;

        std.debug.print("freref:\n {d} ops/ms\n", .{ops_per_ms_q});
    }

    {
        const buf = try std.heap.page_allocator.alloc(i32, capacity);
        defer std.heap.page_allocator.free(buf);

        var ring = spsc_ring.Ring(i32).init(buf);

        var reader2 = try std.Thread.spawn(.{}, spscRingReadWorker, .{ &ring, total_rounds });

        const start_ns_r: i128 = std.time.nanoTimestamp();

        var i_r: u64 = 0;
        while (i_r < total_rounds) : (i_r += 1) {
            while (!ring.enqueue(@intCast(i_r))) {
                std.atomic.spinLoopHint();
            }
        }

        reader2.join();

        const end_ns_r: i128 = std.time.nanoTimestamp();
        const elapsed_ns_r: u128 = @intCast(end_ns_r - start_ns_r);
        const ops_per_ms_r: u128 = (@as(u128, total_rounds) * 1_000_000) / elapsed_ns_r;

        std.debug.print("cdolan:\n {d} ops/ms\n", .{ops_per_ms_r});
    }
}
