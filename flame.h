#ifndef FLAME_H
#define FLAME_H
#include "common.h"
#include "damage.h"
#include <chrono>
//see http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2661.htm
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::high_resolution_clock Clock;

class Flame : public Damage
{
public:
    Flame(Point location, Size s, int deltaX, int deltaY) : Damage::Damage(0, location, s) { //constructor
        creation = Clock::now();
        if ((deltaX == 0) && (deltaY == 0)) { //prevents flames from getting "stuck"
            deltaX = (randomInt(0,1) == 1) ? 1 : -1;
            deltaY = (randomInt(0,1) == 1) ? 1 : -1;
        }
        printf("Flame created; Delta X: %d, Delta Y: %d\n",deltaX,deltaY);
        this->deltaMatrix = {
            deltaX, deltaY,
            deltaX, deltaY,
            deltaX, deltaY,
            deltaX, deltaY
        };
    }
    ~Flame()
    {
//        cout << "!!! FLAME DESTRUCTOR CALLED !!!" << endl;
    }
    GLint* getVertices() {
//        printf("flame before gV() l: %d, r: %d, t: %d, b: %d\n",verts[0],verts[4],verts[1],verts[3]);
        Clock::time_point request = Clock::now();
        milliseconds msdiff = std::chrono::duration_cast<milliseconds>(request - creation);
        long diff = msdiff.count();
        for (int m = 0; m < 8; m++) {
////            printf("previous vert position: %d; delta value:%d",this->verts[m],this->deltaMatrix[m]);
            this->verts[m] += (this->deltaMatrix[m] * (diff / 66));
//            printf("; new vert position: %d\n",this->verts[m]);
        }
//        printf("flame returning l: %d, r: %d, t: %d, b: %d\n",verts[0],verts[4],verts[1],verts[3]);
        return this->verts;
    }
    static const bool isFlame = true;
protected:
private:
    Clock::time_point creation;
    int deltaMatrix[8];
};

#endif // FLAME_H
