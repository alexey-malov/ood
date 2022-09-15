#ifndef IQUAKBEHAVIOR_H
#define IQUAKBEHAVIOR_H

struct IQuackBehavior
{
	virtual ~IQuackBehavior(){};
	virtual void Quack() = 0;
};

#endif
