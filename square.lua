--
-- Show how to draw a square, stroke only, with cairoMoon
-- Feed this code to cairoMoon with:
--   
--   cairoMoon square.lua
-- 
-- To draw. 
--
-- Copyright (C) 2013  Inori Sakura <inorindesu@gmail.com>
-- 
-- This work is free. You can redistribute it and/or modify it under the
-- terms of the Do What The Fuck You Want To Public License, Version 2,
-- as published by Sam Hocevar. See the COPYING file for more details.

outputPath = "/tmp/square.pdf"
stroke = true
fill = false

function drawPath()
  moveTo(30, 30)
  lineTo(40, 30)
  lineTo(40, 40)
  lineTo(30, 40)
  close()
end
