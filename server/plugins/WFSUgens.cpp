/*
	Some WFS and Sync UGens for SC3 by Jan Trutzschler (c) sampleAndHold.org 2006
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "SC_PlugIn.h"

static InterfaceTable *ft;


struct ZeroCrossingCounter : public Unit
{
	float mLevel, m_prevfrac, m_previn;
	int32 mCounter;
};


struct MultiUnPause : public Unit
{
	int32 mCounter, mRange;
};

extern "C"
{
	void load(InterfaceTable *inTable);
	void ZeroCrossingCounter_Ctor(ZeroCrossingCounter *unit);
	void ZeroCrossingCounter_next_a(ZeroCrossingCounter *unit, int inNumSamples);
	void MultiUnPause_Ctor(MultiUnPause *unit);
	void MultiUnPause_next_k(MultiUnPause *unit, int inNumSamples);
	void MultiUnPause_next_a(MultiUnPause *unit, int inNumSamples);
}



void ZeroCrossingCounter_Ctor(ZeroCrossingCounter *unit)
{
	SETCALC(ZeroCrossingCounter_next_a);

	unit->m_prevfrac = 0.f;
	unit->m_previn = ZIN0(0);
	ZOUT0(0) = unit->mLevel = 0.f;
	unit->mCounter = 0;
}

void ZeroCrossingCounter_next_a(ZeroCrossingCounter *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in = ZIN(0);
	float previn = unit->m_previn;
	float prevfrac = unit->m_prevfrac;
	float level = unit->mLevel;
	long counter = unit->mCounter;
	
	LOOP(inNumSamples,
		float curin = ZXP(in);
		if (previn <= 0.f && curin > 0.f) {
			counter++;
		}
		ZXP(out) = counter;
		previn = curin;
	);
	
	unit->m_previn = previn;
	unit->mCounter = counter;	
}

void MultiUnPause_Ctor(MultiUnPause *unit)
{
	SETCALC(MultiUnPause_next_a);
	unit->mCounter = ZIN0(0);
	ZOUT0(0) = ZIN0(0);
	
}

//fixme range is now:
//in: 1 -> free:	1 ... 100
//  2 -> free: 101
//  3 -> free: 103
// should be
//	in : 1 -> free:  1 .. 100
//	in: 2 -> 101 ... 
//
void MultiUnPause_next_k(MultiUnPause *unit, int inNumSamples)
{

	float in = ZIN0(0);
	int count = (int32) in;
	int range = (int) ZIN0(1);
	count = count * range;
	if(count != unit->mCounter)
	{
		for(int i=count; i < (range + count); i++)
		{
			Node *node = SC_GetNode(unit->mWorld, i);
			if (node) {
				NodeRun(node, 1);
			}

		}
		unit->mCounter = count;
	}
	ZOUT0(0) = in;
	
}

void MultiUnPause_next_a(MultiUnPause *unit, int inNumSamples)
{

	float *input = ZIN(0);

	LOOP(inNumSamples,

		float in = ZXP(input);
		int count = (int32) in;
		int range = (int) ZIN0(1);
		
		count = count * range;
		
		if(count != unit->mCounter)
		{
			int range = (int) ZIN0(1);
			for(int i=count; i < (range + count); i++)
			{
				Node *node = SC_GetNode(unit->mWorld, i);
				if (node) {
					NodeRun(node, 1);
				}

			}
			unit->mCounter = count;
		}
	)
//	ZOUT0(0) = in;
	
}


void load(InterfaceTable *inTable)
{
	ft = inTable;

	DefineSimpleUnit(ZeroCrossingCounter);
	DefineSimpleUnit(MultiUnPause);
}
