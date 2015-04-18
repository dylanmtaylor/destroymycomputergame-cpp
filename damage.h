#include "common.h"
#include <iostream>
#include <list>
#ifndef DAMAGE_H
#define DAMAGE_H
using namespace std;

class Damage
{
    // The List STL template requires overloading operators =, == and <.
public:
    Damage(GLuint texID, Point loc, Size s)
    {
        this->_s = s;
        this->tID = texID;
        short left = (loc.x - (s.w / 2));
        short right = (loc.x + (s.w / 2));
        short top = (loc.y - (s.h / 2));
        short bottom = (loc.y + (s.h / 2));

        printf("adding damage at l: %d, r: %d, t: %d, b: %d, tid: %d, s: (%d, %d) \n",left,right,top,bottom,tID, _s.w,_s.h);
        this->verts =  {
            left, top,
            left, bottom,
            right, top,
            right, bottom
        };
        printf("added at l: %d, r: %d, t: %d, b: %d, tid: %d, s: (%d, %d) \n",this->verts[0],this->verts[4],this->verts[1],this->verts[3],this->tID,this-> _s.w,this->_s.h);
    }
    GLint* getVertices() {
//        printf("returning l: %d, r: %d, t: %d, b: %d, tid: %d, s: (%d, %d) \n",this->verts[0],this->verts[4],this->verts[1],this->verts[3],tID, this->_s.w,this->_s.h);
        return this->verts;
    }
    GLuint getTextureID() {
        return tID;
    }
    static const bool isFlame = false;
    ~Damage() {
//        cout << "!!! DAMAGE DESTRUCTOR CALLED !!!" << endl;
    };
protected:
    GLuint tID;
    GLint verts[8];
    Size _s;
private:
};

#endif // DAMAGE_H
