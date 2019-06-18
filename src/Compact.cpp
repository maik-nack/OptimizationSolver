#include <stdint.h>
#include <math.h>
#include <iostream>
#include <qvector.h>
#include <qlist.h>

#include "ILog.h"
#include "ICompact.h"

#define nullptr 0

namespace {
	class Compact : public ICompact
	{
	public:
		class CompactIterator : public ICompact::IIterator
		{
		public:
			CompactIterator(Compact const *compact, unsigned pos, IVector *step = nullptr);

			// IIterator intarface methods:
			int setStep(IVector const* const step);
			int doStep();

			// other utility methods:
			IVector* getPoint() const;

		private:
			Compact const* _compact;	// compact for iterating
			IVector *_step;				// step (only for non-default behaviour)
			unsigned _pos;				// current position in compact
		}; // end CompactIterator

		static unsigned const MAX_POINTS_AMOUNT = UINT_MAX;
		static unsigned const PRECISION_DIVIDER = 1000;

		unsigned _dim;						 // dimension of vectors in compact
		unsigned _pointsAmount;				 // total amount of points in compact
		IVector *_pointBegin;				 // "bottom-left" corner
		IVector *_pointEnd;					 // "top-right" corner
		IVector *_samplingValues;			 // values of distance between the points by every axis
		QVector<unsigned> _samplingCounters; // amounts of points by every axis
		QList<CompactIterator*> _iterators;	 // list of iterators

		Compact(IVector *begin, IVector *end, IVector *samplingValues, QVector<unsigned> &samplingCounters);
		~Compact();

		// ICompact intarface methods:
		ICompact* clone() const;

		int getId() const;
		int getNearestNeighbor(IVector const* vec, IVector *& nn) const;
		int isContains(IVector const* const vec, bool& result) const;
		int isSubSet(ICompact const* const other) const;

                IIterator* begin(IVector const* const step);
                IIterator* end(IVector const* const step = 0);
		int getByIterator(IIterator const* pIter, IVector*& pItem) const;
		int deleteIterator(IIterator *pIter);

		// other utility methods:
		int isSamplingContains(IVector const *vec, bool& result) const;
		int checkStepCorrectness(IVector const *step) const;
		bool vectorPrecisionEquals(IVector const *v1, IVector const *v2) const;
		int findIterator(IIterator const *iterator) const;

		int getIndexByPoint(IVector const *vec, unsigned &result) const;
		IVector* getPointByIndex(unsigned index) const;
	};// end Compact
}// end anonymous namespace


Compact* defaultSamplingCreate(IVector *begin, IVector *end, IVector *residual)
{
	unsigned dim = begin->getDim();
	unsigned defaultCounter = static_cast<unsigned>(pow(Compact::MAX_POINTS_AMOUNT, 1.0 / dim));
	QVector<unsigned> samplingCounters(static_cast<int>(dim), defaultCounter);
	if (residual->multiplyByScalar(1.0 / defaultCounter) != ERR_OK)
	{
		ILog::report("defaultSamplingCreate: failed to multiply residual by scalar\n");
		return nullptr;
	}
	return new(std::nothrow) Compact(begin, end, residual, samplingCounters);
} // end defaultSamplingCreate

Compact* nonDefaultSamplingCreate(IVector *begin, IVector *end, IVector *residual, IVector const *step)
{
	unsigned dim = begin->getDim();
	double coord = 0.0, pointsAmount = 1.0, roundedCounter;
	QVector<unsigned> samplingCounters(static_cast<int>(dim));

	// check sampling correctness
	for (unsigned i = 0; i < dim; i++)
	{
		if (step->getCoord(i, coord) != ERR_OK)
		{
			ILog::report("nonDefaultSamplingCreate: failed to get coord from 'step' param\n");
			return nullptr;
		}
		if ((roundedCounter = round(coord)) < 1.0)
		{
			ILog::report("nonDefaultSamplingCreate: some of 'step' coordinates are less than 1.0\n");
			return nullptr;
		}
		samplingCounters[i] = static_cast<unsigned>(roundedCounter);
		pointsAmount *= roundedCounter;
	}
	if (pointsAmount > static_cast<double>(Compact::MAX_POINTS_AMOUNT))
	{
		ILog::report("nonDefaultSamplingCreate: the amount of points is bigger than max amount\n");
		return nullptr;
	}// end check sampling correctness

	for (unsigned i = 0; i < dim; i++)
	{
		if (residual->getCoord(i, coord) != ERR_OK)
		{
			ILog::report("nonDefaultSamplingCreate: failed to get coord from residual\n");
			return nullptr;
		}
		if (residual->setCoord(i, coord / samplingCounters[i]) != ERR_OK)
		{
			ILog::report("nonDefaultSamplingCreate: failed to set coord to residual\n");
			return nullptr;
		}
	}
	return new(std::nothrow) Compact(begin, end, residual, samplingCounters);
}// end nonDefaultSamplingCreate

ICompact* ICompact::createCompact(IVector const* const begin, IVector const* const end, IVector const* const step)
{
	unsigned dimension;
	double coordBegin = 0.0, coordEnd = 0.0;
	IVector *beginClone, *endClone, *residual;
	ICompact *compact;

	if (!begin || !end)
	{
		ILog::report("ICompact::createCompact: nullptr 'begin' or 'end' params\n");
		return nullptr;
	}
	if ((dimension = begin->getDim()) != end->getDim())
	{
		ILog::report("ICompact::createCompact: dimesions mismatch in 'begin' or 'end' params\n");
		return nullptr;
	}
	for (unsigned i = 0; i < dimension; i++)
	{
		if (begin->getCoord(i, coordBegin) != ERR_OK || end->getCoord(i, coordEnd) != ERR_OK)
		{
			ILog::report("ICompact::createCompact: failed to get coords from 'begin' or 'end' params\n");
			return nullptr;
		}
		if (coordBegin > coordEnd)
		{
			ILog::report("ICompact::createCompact: some 'begin'' coordinates are greater then 'end'\n");
			return nullptr;
		}
	}

	if (!(beginClone = begin->clone()))
	{
		ILog::report("ICompact::createCompact: failed to clone 'begin' param\n");
		return nullptr;
	}
	if (!(endClone = end->clone()))
	{
		ILog::report("ICompact::createCompact: failed to clone 'end' param\n");
		delete beginClone;
		return nullptr;
	}
	if (!(residual = IVector::subtract(end, begin)))
	{
		ILog::report("ICompact::createCompact: failed to subtract 'begin' from 'end'\n");
		delete beginClone;
		delete endClone;
		return nullptr;
	}

	if (!step)
	{
		compact = defaultSamplingCreate(beginClone, endClone, residual);
	}
	else
	{
		if (step->getDim() != dimension)
		{
			ILog::report("ICompact::createCompact: dimesions mismatch in 'step' param\n");
			delete beginClone;
			delete endClone;
			delete residual;
			return nullptr;
		}
		compact = nonDefaultSamplingCreate(beginClone, endClone, residual, step);
	}
	if (!compact)
	{
		ILog::report("ICompact::createCompact: failed to create compact\n");
		delete beginClone;
		delete endClone;
		delete residual;
	}
	return compact;
}// end factory method


Compact::Compact(IVector *begin, IVector *end, IVector *samplingValues, QVector<unsigned> &samplingCounters) : _samplingCounters(samplingCounters)
{
	_pointBegin = begin;
	_pointEnd = end;
	_samplingValues = samplingValues;
	_dim = begin->getDim();
	_pointsAmount = 1;
	for (unsigned i = 0; i < _dim; i++)
	{
		_pointsAmount *= _samplingCounters[i];
	}
}

Compact::~Compact()
{
	delete _pointBegin;
	delete _pointEnd;
	delete _samplingValues;
	for (int i = 0; i < _iterators.count(); i++)
	{
		if (_iterators[i])
		{
			delete _iterators[i];
		}
	}
}

ICompact* Compact::clone() const
{
	double *counters = new(std::nothrow) double[_dim];
	IVector *step;
	ICompact *compact;
	if (!counters)
	{
		ILog::report("ICompact::clone: failed with memory allocation\n");
		return nullptr;
	}
	for (unsigned i = 0; i < _dim; i++)
	{
		counters[i] = _samplingCounters[i];
	}
	step = IVector::createVector(_dim, counters);
	if (!step)
	{
		ILog::report("ICompact::clone: failed to create step vector, continue with default sampling\n");
	}
	compact = createCompact(_pointBegin, _pointEnd, step);
	delete[] counters;
	delete step;
	return compact;
}// end clone

int Compact::getId() const
{
	return ICompact::INTERFACE_0;
}

int Compact::getNearestNeighbor(IVector const* vec, IVector *& nn) const
{
	double coordBegin = 0.0, coordEnd = 0.0, coordVec = 0.0, samplingVal = 0.0;
	IVector *neighbor;
	if (!vec)
	{
		ILog::report("ICompact::getNearestNeighbor: nullptr in 'vec' param\n");
		return ERR_WRONG_ARG;
	}
	if (vec->getDim() != _dim)
	{
		ILog::report("ICompact::getNearestNeighbor: dimensions mismatch in 'vec' param\n");
		return ERR_DIMENSIONS_MISMATCH;
	}

	double *nnValues = new(std::nothrow) double[_dim];
	if (!nnValues)
	{
		ILog::report("ICompact::getNearestNeighbor: failed with memory allocation\n");
		return ERR_MEMORY_ALLOCATION;
	}

	for (unsigned i = 0; i < _dim; i++)
	{
		if (_pointBegin->getCoord(i, coordBegin) != ERR_OK || _pointEnd->getCoord(i, coordEnd) != ERR_OK)
		{
			ILog::report("ICompact::getNearestNeighbor: failed to get coords from '_pointBegin' or from '_pointEnd'\n");
			delete[] nnValues;
			return ERR_ANY_OTHER;
		}
		if (_samplingValues->getCoord(i, samplingVal) != ERR_OK || vec->getCoord(i, coordVec) != ERR_OK)
		{
			ILog::report("ICompact::getNearestNeighbor: failed to get coords from '_samplingValues' or from 'vec'\n");
			delete[] nnValues;
			return ERR_ANY_OTHER;
		}

		// if current coordinate is bigger than the begin point
		if (coordVec > coordBegin)
		{
			// if current coordinate is bigger than the end point
			if (coordVec > coordEnd)
			{
				nnValues[i] = coordEnd;
			}
			// if current coordinate is between the begin and the end points
			else
			{
				nnValues[i] = coordBegin + round((coordVec - coordBegin) / samplingVal) * samplingVal;
			}
		}
		// if current coordinate is less (or equals) than the begin point
		else
		{
			nnValues[i] = coordBegin;
		}
	}

	neighbor = IVector::createVector(_dim, nnValues);
	delete[] nnValues;
	if (neighbor)
	{
		nn = neighbor;
		return ERR_OK;
	}
	else
	{
		ILog::report("ICompact::getNearestNeighbor: failed to create neighbor\n");
		return ERR_MEMORY_ALLOCATION;
	}
}// end getNearestNeighbor

int Compact::isContains(IVector const* const vec, bool& result) const
{
	double coordBegin = 0.0, coordEnd = 0.0, coordVec = 0.0;
	if (!vec)
	{
		ILog::report("ICompact::isContains: nullptr in 'vec' param\n");
		return ERR_WRONG_ARG;
	}
	if (vec->getDim() != _dim)
	{
		ILog::report("ICompact::isContains: dimensions mismatch in 'vec' param\n");
		return ERR_DIMENSIONS_MISMATCH;
	}

	for (unsigned i = 0; i < _dim; i++)
	{
		if (_pointBegin->getCoord(i, coordBegin) != ERR_OK || _pointEnd->getCoord(i, coordEnd) != ERR_OK)
		{
			ILog::report("ICompact::isContains: failed to get coords from '_pointBegin' or from '_pointEnd'\n");
			return ERR_ANY_OTHER;
		}
		if (vec->getCoord(i, coordVec) != ERR_OK)
		{
			ILog::report("ICompact::isContains: failed to get coord from 'vec'\n");
			return ERR_ANY_OTHER;
		}
		if (!(coordVec >= coordBegin && coordVec <= coordEnd))
		{
			result = false;
			return ERR_OK;
		}
	}
	result = true;
	return ERR_OK;
} // end isContains

int Compact::isSubSet(ICompact const* const other) const
{
	ILog::report("ICompact::isSubSet: ERR_NOT_IMPLEMENTED\n");
	Q_ASSERT(ERR_NOT_IMPLEMENTED);
	return ERR_NOT_IMPLEMENTED;
}

ICompact::IIterator* Compact::begin(IVector const* const step)
{
	IVector* stepClone;
	if (!step || checkStepCorrectness(step) == ERR_OK)
	{
		if (!(stepClone = step->clone()))
		{
			ILog::report("ICompact::_begin: failed to clone 'step' param\n");
			return nullptr;
		}
		return new(std::nothrow) CompactIterator(this, 0, stepClone);
	}
	ILog::report("ICompact::_begin: not correct 'step' param\n");
	return nullptr;
}

ICompact::IIterator* Compact::end(IVector const* const step)
{
	IVector* stepClone;
	if (!step || checkStepCorrectness(step) == ERR_OK)
	{
		if (!(stepClone = step->clone()))
		{
			ILog::report("ICompact::_end: failed to clone 'step' param\n");
			return nullptr;
		}
		return new(std::nothrow) CompactIterator(this, _pointsAmount - 1, stepClone);
	}
	ILog::report("ICompact::_end: not correct 'step' param\n");
	return nullptr;
}

int Compact::getByIterator(IIterator const* pIter, IVector*& pItem) const
{
	IVector *point;
	int index = findIterator(pIter);
	if (index < 0)
	{
		ILog::report("ICompact::getByIterator: failed to find iterator\n");
		return ERR_WRONG_ARG;
	}
	if (!(point = _iterators[index]->getPoint()))
	{
		ILog::report("ICompact::getByIterator: failed to get point from iterator\n");
		return ERR_ANY_OTHER;
	}
	pItem = point;
	return ERR_OK;
}

int Compact::deleteIterator(IIterator * pIter)
{
	int index = findIterator(pIter);
	if (index < 0)
	{
		ILog::report("ICompact::deleteIterator: failed to find iterator\n");
		return ERR_WRONG_ARG;
	}
	if (_iterators[index])
	{
		delete _iterators[index];
	}
	_iterators.removeAt(index);
	return ERR_OK;
}

int Compact::isSamplingContains(IVector const *vec, bool& result) const
{
	int errCode;
	if (!vec)
	{
		ILog::report("isSampligContains: nullptr in 'vec' param\n");
		return ERR_WRONG_ARG;
	}
	if (vec->getDim() != _dim)
	{
		ILog::report("isSampligContains: dimensions mismatch in 'vec' param\n");
		return ERR_DIMENSIONS_MISMATCH;
	}

	IVector *neighbor = nullptr;
	if ((errCode = getNearestNeighbor(vec, neighbor)) == ERR_OK)
	{
		result = vectorPrecisionEquals(vec, neighbor);
	}
	else
	{
		ILog::report("isSampligContains: failed to get neagers neighbor\n");
	}
	delete neighbor;
	return errCode;
} // end isContains

int Compact::checkStepCorrectness(IVector const *step) const
{
	double coordStep = 0.0, samplingVal = 0.0;
	int errCode;
	if (!step)
	{
		return ERR_WRONG_ARG;
	}
	for (unsigned i = 0; i < _dim; i++)
	{
		if ((errCode = step->getCoord(i, coordStep)) != ERR_OK)
		{
			return errCode;
		}
		if ((errCode = _samplingValues->getCoord(i, samplingVal)) != ERR_OK)
		{
			return errCode;
		}
		/* if at least one of step coordinates is greather then half of sampling
			value, than step is correct (otherwise step is too small and iteraror
			isn't able to move) */
		if (coordStep > samplingVal / 2.0)
		{
			return ERR_OK;
		}
	}
	ILog::report("checkStepCorrectness: too small step\n");
	return ERR_WRONG_ARG;
}// end checkStepCorrectness


bool Compact::vectorPrecisionEquals(IVector const *v1, IVector const *v2) const
{
	double coord1 = 0.0, coord2 = 0.0, samplingVal = 0.0;
	unsigned dimension;
	if (!v1 || !v2)
	{
		return false;
	}
	if ((dimension = v1->getDim()) != v2->getDim())
	{
		return false;
	}
	for (unsigned i = 0; i < dimension; i++)
	{
		if (v1->getCoord(i, coord1) != ERR_OK || v2->getCoord(i, coord2) != ERR_OK)
		{
			ILog::report("vectorPrecisionEquals: failed to get coords from 'v1' or from 'v2'\n");
			return false;
		}
		if (_samplingValues->getCoord(i, samplingVal) != ERR_OK)
		{
			ILog::report("vectorPrecisionEquals: failed to get coord from '_samplingValues'\n");
			return false;
		}
		if (fabs(coord2 - coord1) > samplingVal / PRECISION_DIVIDER)
		{
			return false;
		}
	}
	return true;
} // end vectorPrecisionEquals

int Compact::findIterator(IIterator const *iterator) const
{
	for (int i = 0; i < _iterators.count(); i++)
	{
		if (iterator == dynamic_cast<IIterator*>(_iterators[i]))
		{
			return i;
		}
	}
	ILog::report("findIterator: no iterator found\n");
	return -1;
}


int Compact::getIndexByPoint(IVector const* const vec, unsigned &result) const
{
	bool contains = false;
	unsigned index = 0;
	double coordBegin = 0.0, coordVec = 0.0, samplingVal = 0.0;
	int errCode = isSamplingContains(vec, contains);
	if (errCode != ERR_OK)
	{
		ILog::report("getIndexByPoint: failed to check if compact contains point\n");
		return errCode;
	}
	if (!contains)
	{
		ILog::report("getIndexByPoint: compact doesn't contain point\n");
		return ERR_WRONG_ARG;
	}
	for (unsigned i = 0; i < _dim; i++)
	{
		if (_pointBegin->getCoord(i, coordBegin) != ERR_OK || _samplingValues->getCoord(i, samplingVal) != ERR_OK)
		{
			ILog::report("getIndexByPoint: failed to get coords from '_pointBegin' or from '_samplingValues'\n");
			return ERR_ANY_OTHER;
		}
		if (vec->getCoord(i, coordVec) != ERR_OK)
		{
			ILog::report("getIndexByPoint: failed to get coord from 'vec' param\n");
			return ERR_ANY_OTHER;
		}
		if (i > 0)
		{
			index *= (_samplingCounters)[i];
		}
		index += static_cast<unsigned>(round((coordVec - coordBegin) / samplingVal));
	}
	result = index;
	return ERR_OK;
}// end getIndexByPoint

IVector* Compact::getPointByIndex(unsigned index) const
{
	unsigned currentIndex;
	double beginCoord = 0.0, samplingVal = 0.0;
	double *coords = new(std::nothrow) double[_dim];
	IVector *vec;
	if (!coords)
	{
		ILog::report("getPointByIndex: failed with memory allocation\n");
		return nullptr;
	}
	if (index >= _pointsAmount)
	{
		ILog::report("getPointByIndex: index out of range\n");
		delete[] coords;
		return nullptr;
	}
	for (unsigned i = _dim; i > 0; i--)
	{
		currentIndex = index % (_samplingCounters)[i - 1];
		index /= (_samplingCounters)[i - 1];
		if (_pointBegin->getCoord(i - 1, beginCoord) != ERR_OK || _samplingValues->getCoord(i - 1, samplingVal) != ERR_OK)
		{
			ILog::report("getPointByIndex: failed to get coords from '_pointBegin' or from '_samplingValues'\n");
			delete[] coords;
			return nullptr;
		}
		coords[i - 1] = beginCoord + currentIndex * samplingVal;
	}
	vec = IVector::createVector(_dim, coords);
	if (!vec)
	{
		ILog::report("getPointByIndex: failed to create vector\n");
	}
	delete[] coords;
	return vec;
}// end getPointByIndex

Compact::CompactIterator::CompactIterator(Compact const *compact, unsigned pos, IVector *step) : IIterator(compact, pos, step)
{
	_compact = compact;
	_pos = pos;
	_step = step;
}

int Compact::CompactIterator::setStep(IVector const* const step)
{
	int errCode;
	IVector *tmp;
	if (step)
	{
		if ((errCode = _compact->checkStepCorrectness(step)) != ERR_OK)
		{
			ILog::report("ICompact::IIterator::setStep: not correct 'step' param\n");
			return errCode;
		}
		tmp = step->clone();
		if (!tmp)
		{
			ILog::report("ICompact::IIterator::setStep: failed to clone 'step' param\n");
			return ERR_ANY_OTHER;
		}
	}
	else
	{
		tmp = nullptr;
	}
	if (_step)
	{
		delete _step;
	}
	_step = tmp;
	return ERR_OK;
}// end setStep

int Compact::CompactIterator::doStep()
{
	// if stepping by default behaviour
	if (_step == nullptr)
	{
		if (_pos < _compact->_pointsAmount - 1)
		{
			_pos++;
			return ERR_OK;
		}
		else
		{
			ILog::report("ICompact::IIterator::doStep: step out of range (default behavour)\n");
			return ERR_OUT_OF_RANGE;
		}
	}
	// if stepping by non-default behaviour
	else
	{
		unsigned newPos;
		IVector *currentPoint = _compact->getPointByIndex(_pos), *neighbor = nullptr;
		int errCode;

		if (!currentPoint)
		{
			ILog::report("ICompact::IIterator::doStep: failed to get point by index (non-default behavour)\n");
			return ERR_ANY_OTHER;
		}
		if ((errCode = currentPoint->add(_step)) != ERR_OK)
		{
			ILog::report("ICompact::IIterator::doStep: failed to add step (non-default behavour)\n");
			delete currentPoint;
			return errCode;
		}
		if ((errCode = _compact->getNearestNeighbor(currentPoint, neighbor)) != ERR_OK)
		{
			ILog::report("ICompact::IIterator::doStep: failed to get nearest neighbor (non-default behavour)\n");
			delete currentPoint;
			return errCode;
		}
		if ((errCode = _compact->getIndexByPoint(neighbor, newPos)) != ERR_OK)
		{
			ILog::report("ICompact::IIterator::doStep: failed to get index by point (non-default behavour)\n");
			delete currentPoint;
			delete neighbor;
			return errCode;
		}
		if (newPos == _pos)
		{
			ILog::report("ICompact::IIterator::doStep: step out of range (default behavour)\n");
			errCode = ERR_OUT_OF_RANGE;
		}
		else
		{
			_pos = newPos;
			errCode = ERR_OK;
		}
		delete currentPoint;
		delete neighbor;
		return errCode;
	}
}// end doStep

IVector* Compact::CompactIterator::getPoint() const
{
	return _compact->getPointByIndex(_pos);
}

ICompact::IIterator::IIterator(ICompact const* const compact, int pos, IVector const* const step)
{
}
