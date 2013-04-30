--
-- Show how to draw a filled (without stroke) square with cairoMoon
-- Feed this code to cairoMoon with:
--   
--   cairoMoon square-fillOnly.lua
-- 
-- To draw. 
--
-- Copyright (C) 2013  Inori Sakura <inorindesu@gmail.com>
-- 
-- This work is free. You can redistribute it and/or modify it under the
-- terms of the Do What The Fuck You Want To Public License, Version 2,
-- as published by Sam Hocevar. See the COPYING file for more details.


outputPath = "/tmp/square-fillOnly.pdf"
stroke = false
fill = true

function drawPath()
  moveTo(30, 30)
  lineTo(40, 30)
  lineTo(40, 40)
  lineTo(30, 40)
  close()
end
