/* binding de GLM

g++ lua_glm.cpp -fvisibility=hidden -fPIC -shared -Wall -W -o libglm.so -lluajit-5.1 -DLUAJIT

   manuel:  https://github.com/g-truc/glm/blob/master/manual.md
   ref api: http://glm.g-truc.net/0.9.8/api/index.html

   frustrum, ortho, persp, rotate , translate … :
   https://glm.g-truc.net/0.9.2/api/a00163.html

   $$DATE$$ : lun. 26 février 2018 (20:02:30)
*/

extern "C" {
#ifndef LUAJIT
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#else
#include "luajit-2.0/lua.h"
#include "luajit-2.0/lauxlib.h"
#include "luajit-2.0/lualib.h"
#endif
}
#include<stdio.h>
#include<stdarg.h>

#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/constants.hpp>        // glm::pi
#include <glm/gtc/type_ptr.hpp>         // value_ptr


#define ERROR(a) fprintf(stderr,"[ %s ] @%04d -> %s\n",__FUNCTION__,__LINE__, a); stack_dump(L, "ERROR")

/* cf "Programming in Lua" pp.225) (pp.257 pour PiL 3) */
static void stack_dump (lua_State *L, const char *state)
{
  int i = 0;
  int top = 0;
  if (!L) goto no_stack_dump;

  top = lua_gettop(L);

  printf("\n ** STACK DUMP (top: %d) %s **\n",top, state);

  for (i = 1; i <= top; i++) {  /* repeat for each level */

    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        printf("  \"%s\" @ %d (%d)\n", lua_tostring(L, i), i, i-(top+1) );
        break;

      case LUA_TBOOLEAN:  /* booleans */
        printf("  %s @ %d (%d)\n", lua_toboolean(L, i) ? "true" : "false" ,i, i-(top+1) );
        break;

      case LUA_TNUMBER:  /* numbers */
        printf("  %g @ %d (%d)\n", lua_tonumber(L, i),i, i-(top+1) );
        break;

      default:  /* pas de traitement particulier pour ces types */
        printf("  type [%s] @ %d (%d)\n", lua_typename(L, t), i, i-(top+1) );
        break;

    }
  }

no_stack_dump:
  printf(" ** STACK DUMP : %s **\n\n",L?"END":"ILLEGAL Lua STATE");
}


// retourne l'index du paramètre en cas de problème
int is_err_in_params( lua_State *L, int type, int nb_params)
{
  int is_err = 0;
  for (int i=-nb_params; i<0 && !is_err; i++) {
    if (lua_type(L, i) != type) {
      is_err = nb_params+1+i;
    }
  }
  return is_err;
}


// retourne une table contenant un vec4 pushé par l'api
static void push_vec4(double (&vec4)[4], lua_State *L)
{
  lua_createtable( L, 4/*nb indexs*/, 0/*nb hashes*/);
  for (int i=0; i<4; i++) {
    lua_pushinteger(L,i+1); // décale table en -2
    lua_pushnumber (L,vec4[i]); // décale la table en -3
    lua_settable (L, -3);
  }
}


// Peuple un vec3 à partir des valeurs d'une table sur le haut de la pile Lua
// Retourne 0 si succès.
static int get_vec3( lua_State *L, glm::vec3 &vec3) //, int index)
{
  int is_err = 0;

  if (!lua_istable(L, 1)) {
    is_err =1;
  }
  //int index_clef = lua_gettop(L);
  //printf("index clé: %d",index_clef);

  for (int i=0; i<3 && !is_err; i++) {
    lua_pushinteger(L, i+1); //clé
    //stack_dump(L,"boucle vec3");
    lua_gettable(L, 1); //dépile la clé

    if (!lua_isnumber(L,-1)) {
      is_err = 1;
    } else {
      vec3[i] = (float) lua_tonumber(L, -1);
      lua_pop(L, 1); //Remove the result from lua_gettable
    }
  }

  return is_err;
}


static int test_get_vec3(lua_State *L)
{
  glm::vec3 vec3;
  int is_err = get_vec3( L, vec3);
  if (!is_err) {
    fprintf(stderr, "get_vec3: %f %f %f\n",vec3.x,vec3.y,vec3.z);
  } else {
    fprintf(stderr, "get_vec3: erreur lors de la récupération du vecteur\n");
  }

  return 0;
}

static int test_push_vec4(lua_State *L)
{
  double v[4]={1.1,2.2,3.3,4.4};
  push_vec4(v,L);

  stack_dump(L,"test_push_vec4");
  return 1;
}


// transposition: https://stackoverflow.com/questions/5097291/incorrect-order-of-matrix-values-in-glm
// les shaders peuvent transposer d'eux-même les matrices envoyées comme Uniforms.
// note: ici, la matrice est déjà au format table de Lua
static int transpose_mat4( lua_State *L)
{

  lua_createtable( L, 16/*nb indexs*/, 0/*nb hashes*/); // dest: -1 source: -2)
  for (int i=1; i<5; i++) {
    for (int j=0; j<4; j++) {
      lua_pushinteger( L, ((i-1)*4)+j+1); // index: -1 (table dest: -2 , table source: -3)
      lua_gettable(L, -3); //résultat placé en haut de la pile, à la place de l'index
      double res = lua_tonumber(L,-1);
      lua_pop(L,1); //vire le résultat (il faudra repusher avant l'index)

      lua_pushinteger( L, (j*4)+i); // index: -1 , dest: -2, source: -3
      lua_pushnumber(L, res); // value: -1, index: -2 , dest: -3, source: -4
      lua_settable (L,-3);
    }
  }
  return 1;
}


static void _mat4_to_table( lua_State *L, glm::mat4 &matrice4x4)
{
  lua_createtable( L, 16/*nb indexs*/, 0/*nb hashes*/);
  const float *m = (const float*)glm::value_ptr(matrice4x4);
  for (int i=0; i<16; i++) {
    lua_pushinteger( L,i+1); //index table lua à modifier
    lua_pushnumber( L, m[i]);
    //printf("%d: %f\n",i+1,m[i]);
    lua_settable (L, -3); // la table est en -3
  }
}

// retourne une matrice identité 4x4
static int new_mat4( lua_State *L)
{
  glm::mat4 matrice4x4 = glm::mat4(1.0);
  _mat4_to_table( L, matrice4x4);

  return 1;
}

// converti un userdata mat4 en une table Lua, et retourne cette table
static int mat4_to_table( lua_State *L)
{
  glm::mat4 *mat4 = (glm::mat4 *) lua_touserdata(L, -1);
  //TODO tester si on avait un truc sur la pile, quand même…
  _mat4_to_table( L, *mat4);
  return 1;
}


glm::mat4 get_matrix4_or_identity( lua_State *L, int matrix_index)
{
  glm::mat4 mat4;
  if (lua_isuserdata( L, matrix_index)) {
    mat4 = *(glm::mat4 *) lua_touserdata( L, matrix_index);
  } else {
    mat4 = glm::mat4(1.0);
  }

  return mat4;
}


template <class T>
void push_userdata( lua_State *L, T &data)
{
    T *userdata = (glm::mat4 *) lua_newuserdata(L, sizeof(T));
    memcpy( userdata, glm::value_ptr(data), sizeof(T));
}

// Entrée: vecteur de translation
//         (optionnel) une matrice au format glm::mat4 (userdate)
// Sortie: (userdata) matrice de translation
static int translate_mat4( lua_State *L)
{
  glm::vec3 v3;
  //stack_dump(L,"translate");
  int col_in_err = get_vec3( L,v3);

  if (!col_in_err) {
    glm::mat4 mat4 = get_matrix4_or_identity( L,2);

    mat4 = glm::translate( mat4, v3);

    push_userdata<glm::mat4>( L, mat4);
    lua_pushnil(L); // pas d'erreur
  } else {
    lua_pushnil(L);
    lua_pushfstring(L, "translate_mat4: invalid parameter %d : %s",col_in_err,lua_tostring(L,col_in_err)); // error
  }
  return 2;
}


// Entrée: vecteur indiquant les axes à transformer
//         angle de transformation (en radian)
//         (optionnel) une matrice au format glm::mat4 (userdate)
// Sortie: (userdata) matrice de rotation
static int rotate_mat4 ( lua_State *L)
{
  glm::vec3 v3;
  int col_in_err = get_vec3 ( L,v3);

  if (col_in_err) {
    lua_pushnil(L);
    lua_pushfstring(L, "rotate_mat4: invalid vec3 parameter %d : %s",col_in_err,lua_tostring(L,col_in_err)); // error
  }

  if (!col_in_err) {
    if (lua_isnumber( L,2)) {
      float angle = (float) lua_tonumber( L,2);
      glm::mat4 mat4 = get_matrix4_or_identity( L,3);

      mat4 = glm::rotate( mat4, angle, v3);

      push_userdata<glm::mat4>( L, mat4);
      lua_pushnil(L); // pas d'erreur

    } else {
      lua_pushnil(L);
      lua_pushstring(L, "rotate_mat4: invalid angle parameter");
      col_in_err=1;
    }
  }

  return 2;
}


// retourne une matrice perspective.
// On retourne directement une table Lua, car il n'y aura pas d'autre opération depuis l'API sur cette matrice.
// en cas d'erreur dans les params, renvoie nil et une chaine contenant l'index et la valeur du param en erreur
static int persp_mat4( lua_State *L)
{
  int col_in_error = is_err_in_params( L, LUA_TNUMBER, 4);
  if ( !col_in_error) {
    float far = lua_tonumber(L,-1);
    float near = lua_tonumber(L,-2);
    float aspect = lua_tonumber(L,-3);
    float angle = lua_tonumber(L,-4); // reçu en degrés

    //fprintf(stderr,"[persp_mat4] angle: %f, aspect: %f, near: %f, far: %f\n",angle,aspect,near,far);
    glm::mat4 proj_mat = glm::perspective(angle, aspect, near, far);
    _mat4_to_table( L, proj_mat); // push matrix in table
    lua_pushnil(L); // no error
  } else {
    lua_pushnil(L); // no matrix
    lua_pushfstring(L, "pers_mat4: invalid paramereter %d : %s",col_in_error,lua_tostring(L,col_in_error)); // error
  }
  return 2;
}





static const luaL_Reg glm_methods[] = {
  {"new_mat4", new_mat4},
  {"persp_mat4", persp_mat4},
  {"mat4_to_table", mat4_to_table},
  {"translate", translate_mat4},
  {"rotate", rotate_mat4},
  {"transpose_mat4", transpose_mat4},
  {"test_get_vec3",test_get_vec3}, //à virer quand ça fonctionnera
  {"test_push_vec4", test_push_vec4}, // idem ci-dessus
  {0,0}
};

/*
static const luaL_Reg glm_metamethods[] = {
        {"__gc", gc},
        //{"__tostring", Truc_tostring},
        {0, 0}
};
*/


extern "C" {
#ifdef __linux
  __attribute__((visibility("default"))) int luaopen_libglm(lua_State *L)
#else
   __declspec(dllexport) int luaopen_libglm(lua_State *L)
#endif
    {
#ifndef LUAJIT
      luaL_newlibtable(L, glm_methods);
      luaL_setfuncs(L, glm_methods, 0);
      return 1;
#else
      luaL_register (L, "mini", glm_methods);
      return 1;
#endif
    }
}



#ifdef TEST_GLM
int main(void)
{
  glm::mat4 matrice4x4 = glm::mat4(1.0);
  matrice4x4 = glm::translate( matrice4x4, glm::vec3(0,0,-10) );
  matrice4x4 = glm::transpose(matrice4x4); //passage au format colonnes utilisé par opengl
  const float *m = (const float*)glm::value_ptr(matrice4x4);
  for (int i=0; i<4; i++) {
    printf("%02f %02f %02f %02f\n",m[i*4],m[i*4+1],m[i*4+2],m[i*4+3]);
  }

  return 0;
}
#endif
