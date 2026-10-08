-- test.lua
-- CPU usage per thread – only objects of type 1 (THREAD) are taken into account
-- Works with 16-bit or 32-bit TraceX timestamps

local threads = {}              -- pointer → { total = 0 }
local previous_thread = nil
local previous_ts     = nil

-- Special TraceX values
local ISR_THREAD  = 0xFFFFFFFF
local INIT_THREAD = 0xF0F0F0F0
local UNUSED      = 0

-- timestamp_mask is set by C from the header (default to 32-bit if missing)
local mask = timestamp_mask or 0xFFFFFFFF

local function delta_ticks(prev, curr)
    -- Correct wrap-around arithmetic
    return (curr - prev) & mask
end

-- Helper: returns true only if the pointer is a known THREAD (type == 1)
local function is_thread(ptr)
    if not ptr then
        return false
    end
    local obj = objects and objects[ptr]
    return obj and obj.type == 1
end

function on_event(eventId, threadPointer, threadPriority,
                  info1, info2, info3, info4, timeStamp)

    local current = threadPointer
    if current == ISR_THREAD or current == INIT_THREAD or current == UNUSED then
        current = nil
    end

    -- Only credit time if the previous running entity was a real THREAD
    if previous_thread and previous_ts and is_thread(previous_thread) then
        local delta = delta_ticks(previous_ts, timeStamp)

        if delta > 0 then
            local t = threads[previous_thread]
            if not t then
                t = { total = 0 }
                threads[previous_thread] = t
            end
            t.total = t.total + delta
        end
    end

    previous_thread = current
    previous_ts     = timeStamp
end

function finalize()
    -- nothing needed
end

function get_result()
    local result = {
        name = "cpu_usage_per_thread",
        description = "CPU time and percentage used by each thread",
        suggested_view = "timeline",
        columns = {
            { name = "object_ptr",  type = "uint32" },
            { name = "name",        type = "string" },
            { name = "total_time",  type = "uint64", unit = "ticks" },
            { name = "cpu_percent", type = "double", unit = "%" }
        },
        rows = {}
    }

    -- Total time = sum of everything we credited to threads
    local total_elapsed = 0
    for ptr, t in pairs(threads) do
        if is_thread(ptr) then
            total_elapsed = total_elapsed + t.total
        end
    end
    if total_elapsed == 0 then
        total_elapsed = 1
    end

    for ptr, t in pairs(threads) do
        -- Final filter: only emit rows for type-1 objects
        if is_thread(ptr) then
            local obj  = objects[ptr]
            local name = (obj and obj.name) or string.format("0x%X", ptr)
            local percent = (t.total * 100.0) / total_elapsed

            table.insert(result.rows, {
                ptr,
                name,
                t.total,
                percent
            })
        end
    end

    return result
end