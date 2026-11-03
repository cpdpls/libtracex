-- event_count_by_id.lua
--
-- Counts occurrences of each event_id in the trace.
-- Also splits counts by context: THREAD / ISR / INIT / OTHER.
--
-- Required globals (may be filled by C):
--   objects = {}
--   events  = {}   -- kept for loader contract; unused here

objects = {}
events  = {}

local ISR_THREAD  = 0xFFFFFFFF
local INIT_THREAD = 0xF0F0F0F0
local UNUSED      = 0

-- event_id → { total, on_thread, on_isr, on_init, on_other }
local counts = {}

local function ctx_bucket(thread_pointer)
    if thread_pointer == ISR_THREAD then
        return "on_isr"
    elseif thread_pointer == INIT_THREAD then
        return "on_init"
    elseif thread_pointer == UNUSED or thread_pointer == nil then
        return "on_other"
    end
    local obj = objects[thread_pointer]
    if obj and obj.type == 1 then
        return "on_thread"
    end
    return "on_other"
end

function on_event(event_id, thread_pointer, thread_priority,
                  info1, info2, info3, info4, timestamp)
    local c = counts[event_id]
    if not c then
        c = {
            total     = 0,
            on_thread = 0,
            on_isr    = 0,
            on_init   = 0,
            on_other  = 0
        }
        counts[event_id] = c
    end

    c.total = c.total + 1
    local bucket = ctx_bucket(thread_pointer)
    c[bucket] = c[bucket] + 1
end

function finalize()
    -- nothing
end

function get_result()
    local result = {
        name = "event_count_by_id",
        description = "Number of times each event ID occurs, by execution context",
        suggested_view = "table",
        columns = {
            { name = "event_id",  type = "uint32" },
            { name = "total",     type = "uint64" },
            { name = "on_thread", type = "uint64" },
            { name = "on_isr",    type = "uint64" },
            { name = "on_init",   type = "uint64" },
            { name = "on_other",  type = "uint64" },
            { name = "percent",   type = "double", unit = "%" }
        },
        rows = {}
    }

    local grand = 0
    for _, c in pairs(counts) do
        grand = grand + c.total
    end
    if grand == 0 then
        grand = 1
    end

    -- stable order: sort event ids
    local ids = {}
    for id, _ in pairs(counts) do
        table.insert(ids, id)
    end
    table.sort(ids)

    for _, id in ipairs(ids) do
        local c = counts[id]
        local pct = (c.total * 100.0) / grand
        table.insert(result.rows, {
            id,
            c.total,
            c.on_thread,
            c.on_isr,
            c.on_init,
            c.on_other,
            pct
        })
    end

    return result
end