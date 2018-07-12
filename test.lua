#!/usr/bin/env luajit
-- Pour tester la lib GLM-Lua
-- $$DATE$$ : lun. 26 février 2018 (20:04:54)
-- Attention , car compilé avec la lib luajit5-1 pour utiliser avec Löve2D
-- Il faut en tenir compte quand on lance l'interpréteur (ou compiler sans lier à la lib lua)

glm=require"libglm"

local function print_matrix(name,m)
  print(name)
  if m == nil then
    print("matrix nil : rien à dumper")
  else
    -- si on a passé un userdata GLM, on convertit localement
    -- en table Lua
    if type(m) == "userdata" then m = glm.mat4_to_table(m) end
    for i = 0,3 do
      for j = 1,4 do
        local cell = m[i*4+j]
        io.write(string.format("%10.4f%s",cell,j%4==0 and "\n" or ""))
      end
    end
    print()
  end
end


function main()
  m=glm.new_mat4()
  m[15] = -10 --translation z
  print_matrix("translation z",m)

  mat4_transposed = glm.transpose_mat4(m)
  print_matrix("transposée de translation z",mat4_transposed)

  local fovy,aspect,near,far = math.rad(45), 1024/768 ,0.1 ,1000
  matrice_perspective, errmsg = glm.persp_mat4( fovy, aspect, near, far)
  print(errmsg and errmsg or "")
  print_matrix("matrice perspective", matrice_perspective)

  glm.test_get_vec3( { 1.1,2.2,3.3 } )

  local mat_trans, errmsg = glm.translate( { 0,0,-10 } )
  if errmsg then print("glm.translate",errmsg) end
  print_matrix("mat_trans", mat_trans)

  mat_trans, errmsg = glm.translate( {0,-5,0}, mat_trans)
  print_matrix("mat_trans (ajout trans -5 sur y)", mat_trans)

  mat_rot, errmsg = glm.rotate( { 0,0,1 }, math.pi/2)
  print_matrix("mat_rot",mat_rot)
end

main()
