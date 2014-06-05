#include "atackStruct.h"
#include <QtGui>
bool atack::operator<(const atack & b) const
{
	int x1=target->split("|",QString::SkipEmptyParts).at(0).toInt();
	int y1=target->split("|",QString::SkipEmptyParts).at(1).toInt();
	int x2=sender->split("|",QString::SkipEmptyParts).at(0).toInt();
	int y2=sender->split("|",QString::SkipEmptyParts).at(1).toInt();
	double lenght1=qSqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
	x1=b.target->split("|",QString::SkipEmptyParts).at(0).toInt();
	y1=b.target->split("|",QString::SkipEmptyParts).at(1).toInt();
	x2=b.sender->split("|",QString::SkipEmptyParts).at(0).toInt();
	y2=b.sender->split("|",QString::SkipEmptyParts).at(1).toInt();
	return lenght1>qSqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}
atack& atack::operator=(atack &b)
{
	sender=b.sender;
	target=b.target;
	return *this;
}
atack::atack()
{
	sender=NULL;
	target=NULL;
}