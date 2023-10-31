Referenced_Coroutines = {}
function Internal_CoroutineCreator(f, ...) -- called by script creator
    --debug.setupvalue(f, 1, sandbox_env)
    local m_coroutine = coroutine.create(f)
    Referenced_Coroutines[f] = {}
    Referenced_Coroutines[f][m_coroutine] = {...}
    return m_coroutine
end
function Internal_CoroutineCreatorAction(f, ...)
    --debug.setupvalue(f, 1, sandbox_env)
    if Referenced_Coroutines[f] ~= nil then
        return nil
    else
        local m_coroutine = coroutine.create(f)
        Referenced_Coroutines[f] = {}
        Referenced_Coroutines[f][m_coroutine] = {...}
        return m_coroutine
    end
end
function Internal_CoroutineRemover(thread)
    Referenced_Coroutines[thread] = nil
end
local success = false
local msg = ''
function OnUpdate() -- called by scriptmanager 
    for f,tb in pairs(Referenced_Coroutines) do
        for co,params in pairs(tb) do
            local status = coroutine.status(co)
            if status ~= 'suspended' then
                tb[co] = nil
                Referenced_Coroutines[f] = nil
            else
                if not params then
                    params = Referenced_Coroutines[co]
                    if not params then
                        params = {}
                    end
                end
                success, msg = coroutine.resume(co, table.unpack(params))
                if not success then
                    tb[co] = nil
                    error(msg, 2)
                end
                if coroutine.status(co) ~= 'suspended' then
                    tb[co] = nil
                    Referenced_Coroutines[f] = nil
                end
            end
        end
    end
end
function Api_Error(message)
    error(message, 3)
end
function Internal_CoroutineRunner(f, co)
    local tb = Referenced_Coroutines[f]
    local params = tb[co]
    success, msg = coroutine.resume(co, table.unpack(params))
    if not success then
        Referenced_Coroutines[f] = nil
        error(msg, 3)
    end
    if coroutine.status(co) ~= 'suspended' then
        Referenced_Coroutines[f] = nil
    end
end

function wait(x)
  local startTime = InternalTime
  local targetTime = InternalTime + x
  while InternalTime < targetTime do
    coroutine.yield()
  end
  return InternalTime - startTime
end

sandbox_env = {
  ipairs = ipairs,
  next = next,
  pairs = pairs,
  pcall = pcall,
  tonumber = tonumber,
  tostring = tostring,
  type = type,
  unpack = unpack,
  coroutine = { create = coroutine.create, resume = coroutine.resume,
      running = coroutine.running, status = coroutine.status,
      wrap = coroutine.wrap },
  string = { byte = string.byte, char = string.char, find = string.find,
      format = string.format, gmatch = string.gmatch, gsub = string.gsub,
      len = string.len, lower = string.lower, match = string.match,
      rep = string.rep, reverse = string.reverse, sub = string.sub,
      upper = string.upper },
  table = { unpack = table.unpack, insert = table.insert, maxn = table.maxn, remove = table.remove,
      sort = table.sort, concat = table.concat },
  math = { abs = math.abs, acos = math.acos, asin = math.asin,
      atan = math.atan, atan2 = math.atan2, ceil = math.ceil, cos = math.cos,
      cosh = math.cosh, deg = math.deg, exp = math.exp, floor = math.floor,
      fmod = math.fmod, frexp = math.frexp, huge = math.huge,
      ldexp = math.ldexp, log = math.log, log10 = math.log10, max = math.max,
      min = math.min, modf = math.modf, pi = math.pi, pow = math.pow,
      rad = math.rad, random = math.random, randomseed = math.randomseed, sin = math.sin, sinh = math.sinh,
      sqrt = math.sqrt, tan = math.tan, tanh = math.tanh },
  os = { clock = os.clock, difftime = os.difftime, time = os.time, date = os.date },
  wait = wait,
  Referenced_Coroutines = Referenced_Coroutines,
  Internal_CoroutineCreator = Internal_CoroutineCreator,
  Internal_CoroutineRemover = Internal_CoroutineRemover,
  Internal_CoroutineRunner = Internal_CoroutineRunner,
  InternalTime = InternalTime,
  OnUpdate = OnUpdate,
}
local result = 'Api:\n'
function printtable(tbl, indent)
    for k,v in pairs(tbl) do
        for i=1,indent do
            result = result .. '   -'
        end
        result = result .. k .. '\n'
        if type(v) == 'table' then
            printtable(v, indent + 1)
        end
    end
end
function printApi()
    printtable(sandbox_env, 0)
    print(result)
end
copyImports(sandbox_env)