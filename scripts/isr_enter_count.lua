-- isr_enter_count.lua
--
-- Counts ISR enters using TraceX event IDs:
--   event_id 3 = ISR Enter  (info2 = ISR Number)
--   event_id 4 = ISR Exit   (info2 = ISR Number)
--
-- Optional: also tracks time between Enter and matching Exit.

objects = {}
events  = {}

local EVENT_ISR_ENTER = 3
local EVENT_ISR_EXIT  = 4

-- isr_number → { enters, exits, ticks }
local isrs = {}

-- stack of open ISR intervals: { id = n, start = ts }
local isr_stack = {}

local total_enters = 0
local total_isr_ticks = 0

local function get_mask()
    return timestamp_mask or 0xFFFFFFFF
end

local function delta_ticks(prev, curr)
    return (curr - prev) & get_mask()
end

local function ensure(id)
    local s = isrs[id]
    if not s then
        s = { enters = 0, exits = 0, ticks = 0 }
        isrs[id] = s
    end
    return s
end

function on_event(event_id, thread_pointer, thread_priority,
                  info1, info2, info3, info4, timestamp)

    if event_id == EVENT_ISR_ENTER then
        local id = info2   -- ISR Number
        local s = ensure(id)
        s.enters = s.enters + 1
        total_enters = total_enters + 1
        table.insert(isr_stack, { id = id, start = timestamp })

    elseif event_id == EVENT_ISR_EXIT then
        local id = info2
        local s = ensure(id)
        s.exits = s.exits + 1

        -- close the matching open enter (nesting-friendly: top of stack)
        if #isr_stack > 0 then
            local top = isr_stack[#isr_stack]
            table.remove(isr_stack)
            local d = delta_ticks(top.start, timestamp)
            if d > 0 then
                local t = ensure(top.id)
                t.ticks = t.ticks + d
                total_isr_ticks = total_isr_ticks + d
            end
        end
    end
end

function finalize()
    -- open ISRs with no Exit: ignore time (or credit to last ts if you prefer)
    isr_stack = {}
end

function get_result()
    local result = {
        name = "isr_enter_count",
        description = "ISR enter count from event_id 3 (ISR Enter), ISR number in info2",
        suggested_view = "pie",
        columns = {
            { name = "isr_number",  type = "uint32" },
            { name = "enter_count", type = "uint64" },
            { name = "exit_count",  type = "uint64" },
            { name = "enter_pct",   type = "double", unit = "%" },
            { name = "ticks",       type = "uint64", unit = "ticks" },
            { name = "time_pct",    type = "double", unit = "%" }
        },
        rows = {}
    }

    local enters_base = total_enters > 0 and total_enters or 1
    local ticks_base  = total_isr_ticks > 0 and total_isr_ticks or 1

    local ids = {}
    for id, _ in pairs(isrs) do
        table.insert(ids, id)
    end
    table.sort(ids, function(a, b)
        return isrs[a].enters > isrs[b].enters
    end)

    for _, id in ipairs(ids) do
        local s = isrs[id]
        table.insert(result.rows, {
            id,
            s.enters,
            s.exits,
            (s.enters * 100.0) / enters_base,
            s.ticks,
            (s.ticks * 100.0) / ticks_base
        })
    end

    return result
end