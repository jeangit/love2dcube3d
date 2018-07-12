-- véritable test 3D sans Love3D
-- Il faut : love0.11 (ou mieux), glm et qq fonctions helprs (shaderSendMatrix, …)
-- $$DATE$$ : jeu. 12 juillet 2018 (19:45:52)

local glm = require "libglm"
local shaderSendMatrix = require "shaderSendMatrix"


local shader_source = [[
#ifdef VERTEX
  /* attribute vec3 VertexNormal;
  uniform mat4 u_model; //uniform ou extern, même chose pour Löve2D
  uniform mat4 u_viewProj; */

  uniform mat4 projection_matrix;
  uniform mat4 view_matrix;
  //c'est la fonction par défaut pour un vertex shader
  vec4 position(mat4 transform_projection, vec4 vertex_position)
  {
    // The order of operations matters when doing matrix multiplication.
    return projection_matrix * view_matrix * vertex_position;
  }

#endif
]]

local function create_mesh()
  -- https://love2d.org/wiki/Shader_Variables : pour les attributs prédéfinis
  -- si l'attribut n'est pas prédéfini par love, il faut le spécifier dans le shader
  -- ex: attribute vec3 VertexNormal
  local vertex_format = {
  --{"nom attribut", "type", "nombre d'éléments pour cet attribut"}
    {"VertexPosition", "float", 3}, -- The x,y,z position of each vertex.
    {"VertexColor", "float", 4}  -- vec4
  }

  local liste = {
    -- attention à la taille du quad par rapport à la projection
    -- pour utiliser des coordonnées normalisées (-1,1), il faut une
    -- projection qui soit dans les mêmes limites
    { -1,1,0,   1,0,0,.2},  { 1,1,0,    1,0,0,.2},  { 1,-1,0,   1,0,0,.2}, --face avant
    { 1,-1,0,   .8,0,0,.2}, {-1,-1,0,  .8,0,0,.2},  {-1,1,0,   .8,0,0,.2},

    { -1,1,-2,   0,1,0,1},  { 1,1,-2,    0,1,0,1},  { 1,-1,-2,   0,1,0,1}, --face arrière
    { 1,-1,-2,   0,.8,0,1}, {-1,-1,-2,  0,.8,0,1},  {-1,1,-2,   0,.8,0,1},

    { -1,1,-2,   0,0,1,1},  { -1,1,0,    0,0,1,1},  { -1,-1,0,   0,0,1,1}, --face gauche
    { -1,-1,0,   0,0,.8,1}, { -1,-1,-2,  0,0,.8,1},  {-1,1,-2,   0,0,.8,1},

    { 1,1,-2,   0,.6,1,1},  { 1,1,0,    0,.6,1,1},  { 1,-1,0,   0,.6,1,1}, --face droite
    { 1,-1,0,   0,.3,.8,1}, { 1,-1,-2,  0,.3,.8,1},  {1,1,-2,   0,.3,.8,1}
  }


  -- https://love2d.org/wiki/love.graphics.newMesh
  local mesh = love.graphics.newMesh( vertex_format, liste, "triangles", "dynamic" )
  --local mesh = love.graphics.newMesh( vertex_format, liste, "triangles", "dynamic" )

  return mesh

end

local function print_matrix(name,m)
  print(name)
  if m == nil then
    print("matrix in nil")
  else
    for i = 0,3 do
      for j = 1,4 do
        local cell = m[i*4+j]
        io.write(string.format("%10.4f%s",cell,j%4==0 and "\n" or ""))
      end
    end
    print()
  end
end


local function init_matrices()
  local width, height = love.graphics.getDimensions( )
  local fovy,aspect,near,far = math.rad(45), width/height ,0.1 , 1000
  -- une matrice est un simple tableau indexé de n éléments (16 pour une mat4)
  -- ATTENTION, GLSL attends un format colonnes, et non pas lignes
  -- DONC : m[1] à m[4] = première COLONNE de la matrice
  --        m[12] à m[16] = dernière COLONNE de la matrice
  --matrice_perspective = cpml.mat4.from_perspective(fovy, aspect, near, far)
  matrice_perspective, errmsg = glm.persp_mat4( fovy, aspect, near, far)
  print(errmsg and errmsg or "")
  print_matrix("matrice_perspective",matrice_perspective)

  matrice_view = glm.new_mat4()

end


local function create_shader(src)
  local shader = love.graphics.newShader(src)
  local warnings = shader:getWarnings()
  print("shader warnings",warnings)
  return shader
end

local function stats()
  print("getDepthMode", love.graphics.getDepthMode() )
  print("getMeshCullMode", love.graphics.getMeshCullMode() )
end


function love.load()
  -- pas la peine de toucher au depth , on dirait
  -- je le laisse pour référence au cas où nous aurions du z-fighting plus tard
  -- il est bon de savoir qu'on peut changer la précision du depth pour la fenêtre
  -- on peut aussi le changer sur les canvas
  --love.window.setMode(500,500, { depth=24 })
  -- il faut impérativement activer le test de profondeur, sinon ça trace n'importe comment
  love.graphics.setDepthMode( "less", true )
  -- voir également le source « main.lua_depthcanvas » dans ce rep, pour les manips canvas
  shader = create_shader(shader_source)
  mesh = create_mesh()
  --image = love.graphics.newImage("poisson.png")
  --mesh:setTexture(image)

  init_matrices()

  stats()
  default_blendmode = love.graphics.getBlendMode() --alphamultiply
end

local delta = 0
function love.update(dt)
  delta = delta+dt

  matrice_view = glm.translate( {0, math.cos(delta), -15 + math.sin(delta)*10} )
  matrice_view = glm.rotate( {0.7,1,0}, math.sin(delta)*2, matrice_view)
--  matrice_view = glm.mat4_to_table(matrice_view)
--  matrice_view[15]=matrice_view[15]+(math.sin(delta)/10)
end

function love.draw()
  love.graphics.clear(0.3,0.3,0.3)
  love.graphics.setShader(shader)


  -- pour envoyer une matrice 4x4, _avant love0.10_ , elle devait être sous cette forme:
  -- { { table indexée premier vecteur } , … , { table indexée quatrième vecteur } }
  -- Exemple pour une simple translation de -25 en Z :
  -- { { 1,0,0,0 } , { 0,1,0,0 } , { 0,0,1,-25 } , { 0,0,0,1 } }
  -- OpenGL utilise des matrices colonnes: https://www.khronos.org/opengl/wiki/Data_Type_(GLSL)
  -- Ainsi, la translation se retrouve «sur la droite» et non «en bas» de la matrice
  -- il faut préciser lors de l'envoi de la matrice, que c'est une matrice «column» car
  -- tel est le format de OpenGL. Ceci évite de se taper une tranposition préalable.
  -- Comme Löve2D doit extraire les données de la table pour les passer au shader, cela
  -- ne représente pas de traitement supplémentaire (Löve2D remplit simplement différement
  -- le buffer cible).
  shaderSendMatrix(shader, "projection_matrix", "column", matrice_perspective )
  -- à noter que depuis 10.2 , on peut simplement envoyer une seule table qui contient 16 valeurs
  -- de la matrice 4x4 (donc plus besoin de l'appel à «:to_vec4s() » )
  shaderSendMatrix(shader, "view_matrix", "column", matrice_view )
  love.graphics.setBlendMode("alpha")
  love.graphics.draw(mesh)
  love.graphics.setShader()
  love.graphics.setBlendMode(default_blendmode)
end

function love.keypressed (key, scancode, is_repeat)
  if scancode == "escape" then
    love.event.quit()
  end
end
