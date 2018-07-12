# Un cube en 3D avec Löve2D

Il faut compiler libglm.so de cette façon:

`g++ lua_glm.cpp -fvisibility=hidden -fPIC -shared -Wall -W -o libglm.so -lluajit-5.1 -DLUAJIT`

Puis lancer `love` , au moins en version 11.
