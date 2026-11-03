-- stack_usage.lua
-- Stack usage per THREAD (type == 1).
--
-- Object registry (via C → objects[ptr]):
--   param1 = stack start
--   param2 = stack size
--
-- Event: Thread Resume (id 1)
--   info1 = thread ptr
--   info3 = stack ptr
--
-- Assumes stack grows downward: used = param1 - min_sp

objects = {}
events  = {}

local EVENT_THREAD_RESUME = 1

local seen = {}  -- ptr → { min_sp, max_sp, samples }

local function is_thread(ptr)
    local o = objects[ptr]
    return o and o.type == 1
end

function on_event(event_id, thread_pointer, thread_priority,
                  info1, info2, info3, info4, timestamp)
    if event_id ~= EVENT_THREAD_RESUME then
        return
    end

    local thr = info1
    local sp  = info3
    if not is_thread(thr) then
        return
    end

    local s = seen[thr]
    if not s then
        s = { min_sp = sp, max_sp = sp, samples = 0 }
        seen[thr] = s
    end
    if sp < s.min_sp then s.min_sp = sp end
    if sp > s.max_sp then s.max_sp = sp end
    s.samples = s.samples + 1
end

function finalize()
end

function get_result()
    local result = {
        name = "stack_usage_per_thread",
        description = "Stack usage per thread (param1=start, param2=size; Resume info3=SP)",
        suggested_view = "bar",
        columns = {
            { name = "object_ptr", type = "uint32" },
            { name = "name",       type = "string" },
            { name = "stack_size", type = "uint32", unit = "bytes" },
            { name = "used_bytes", type = "uint32", unit = "bytes" },
            { name = "usage_pct",  type = "double", unit = "%" },
            { name = "samples",    type = "uint32" }
        },
        rows = {}
    }

    for ptr, s in pairs(seen) do
        if is_thread(ptr) then
            local obj = objects[ptr]
            local name = (obj and obj.name) or string.format("0x%X", ptr)
            local stack_start = (obj and obj.param1) or 0
            local stack_size  = (obj and obj.param2) or 0

            local used = 0
            local pct = 0.0

            if stack_size > 0 and stack_start > 0 then
                if stack_start >= s.min_sp then
                    used = stack_start - s.min_sp
                end
                if used > stack_size then
                    used = stack_size
                end
                pct = (used * 100.0) / stack_size
            else
                used = (s.max_sp >= s.min_sp) and (s.max_sp - s.min_sp) or 0
                pct = 0.0
            end

            table.insert(result.rows, {
                ptr,
                name,
                stack_size,
                used,
                pct,
                s.samples
            })
        end
    end

    table.sort(result.rows, function(a, b)
        return a[5] > b[5]
    end)

    return result
end