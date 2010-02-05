MAX_X = 31
MAX_Y = 23
SCREEN_W = 256
SCREEN_H = 192
TEXT_W = SCREEN_W / MAX_X
TEXT_H = SCREEN_H / MAX_Y

local profile_start = dslib.profile_start
local profile_end = dslib.profile_end
local wait = dslib.wait

local function execute(file, ...)
  local code,err = loadfile(file)
  if err then
    print(err)
  else
    io.write("#execute: " .. file)
    if ... then
      arg = {...}
      for i,v in ipairs(arg) do
        io.write(" ")
        io.write(v)
      end
    end
    io.write("\n")
    dslib.vram_mode()
    profile_start()
    code()
    profile_end()
  end
end

local function select_list(list)
  local function text_list()
    for i,v in ipairs(list) do
      dslib.text(0, 0, i-1, v)
    end
  end
  local function selected(x, y)
    if x and y then
      local line = math.floor(y / TEXT_H)
      if line < #list then
        return line, list[line+1]
      end
    end
  end

  dslib.text_mode()

  dslib.cleartext(0)
  text_list()
  while true do
    local x, y = dslib.stylus.x, dslib.stylus.y
    local line = selected(x, y)
    if dslib.stylus.released and line then
      return list[line+1]
    elseif dslib.stylus.held and line then
      text_list()
      dslib.texttilecol(0, 2)
      dslib.text(0, 0, line, list[line+1])
      dslib.texttilecol(0, 0)
    elseif dslib.pad.newpress.start then
      dslib.softreset()
    end
    wait()
  end
end

while true do
  local files = {dslib.ls()}
  if files and type(files)=="table" then
    local selected = select_list(files)
    execute(selected)
  end
  wait()
end
