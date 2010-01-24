function execute(file, arguments)
  io.write("#execute: ")
  io.write(file)
  io.write("\n")
  if arguments then
    for i,v in ipairs(arguments) do
      io.write(" ")
      io.write(v)
    end
    io.write("\n")
    arg = arguments
  end
  profile_start()
  dofile(file)
  profile_end()
end
execute("binarytrees.lua", {"10"})
execute("ao.lua")
