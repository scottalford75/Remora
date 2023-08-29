#include "mbed.h"

#include "qeiDriver.h"

QEIdriver::QEIdriver() :
    qeiIndex(NC)
{
    this->init();

}

QEIdriver::QEIdriver(bool hasIndex) :
    hasIndex(hasIndex),
    qeiIndex(NC)
{
    this->init();;
}

void QEIdriver::interruptHandler()
{

}


uint32_t QEIdriver::get()
{
    return false;
}


void QEIdriver::init()
{
    printf("  Initialising hardware QEI module\n");
    printf("        This target does not support a QEI module\n");
}
