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

local glm=require"libglm"
local unpack = table.unpack or unpack
local function shaderSendMatrix(shader,name,...)
	local inargs,outargs,argsnum = {...},{},select("#", ...)
	for i=1,argsnum do
		local curarg = inargs[i]
		if type(curarg)=="userdata" then
			outargs[i]=glm.mat4_to_table(curarg)
		else
			outargs[i]=curarg
		end
	end
	shader:send(name,unpack(outargs,1,argnum))
end
return shaderSendMatrix
