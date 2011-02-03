/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <CoreAudio/AudioHardware.h>
#include <CoreAudio/HostTime.h>

#include "SCBase.h"
#include "VMGlobals.h"
#include "PyrKernel.h"
#include "PyrPrimitive.h"
#include "GC.h"

enum
{
	OUT = 0,
	IN,
	BOTH
};

int listDevices(struct VMGlobals *g, int type)
{
	int numDevices, num = 0;
    PyrSlot *a = g->sp-2;

//	unsigned long count;
    UInt32 count;
    OSStatus err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &count, 0);
	AudioDeviceID *devices = (AudioDeviceID*)malloc(count);
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &count, devices);
	if (err!=kAudioHardwareNoError)
	{
		free(devices);
		return 0;
	}

	numDevices = count / sizeof(AudioDeviceID);

	int i;

	if (type<BOTH)
	{
		for (i=0; i<numDevices; i++)
		{
			Boolean writeable;
			err = AudioDeviceGetPropertyInfo(devices[i], 0, type, kAudioDevicePropertyStreams, &count, &writeable);
			if (err!=kAudioHardwareNoError)
			{
				free(devices);
				return 0;
			}
			if (!count) devices[i] = 0;
			else num++;
		}
	}
	else num = numDevices;

    PyrObject* devArray = newPyrArray(g->gc, num * sizeof(PyrObject), 0, true);
	SetObject(a, devArray);

	int j = 0;
	for (i=0; i<numDevices; i++)
	{
		if (!devices[i]) continue;

		err = AudioDeviceGetPropertyInfo(devices[i], 0, false, kAudioDevicePropertyDeviceName, &count, 0);
		if (err != kAudioHardwareNoError)
		{
			break;
		}

		char *name = (char*)malloc(count);
		err = AudioDeviceGetProperty(devices[i], 0, false, kAudioDevicePropertyDeviceName, &count, name);
		if (err != kAudioHardwareNoError)
		{
			free(name);
			break;
		}

        PyrString *string = newPyrString(g->gc, name, 0, true);
        SetObject(devArray->slots+j, string);
        devArray->size++;
        g->gc->GCWrite(devArray, (PyrObject*)string);

		free(name);
		j++;
	}

	free(devices);
	return 1;
}

int prListAudioDevices(struct VMGlobals *g, int numArgsPushed)
{
	int in = 0;
	int out = 0;
	slotIntVal(g->sp, &out);
	slotIntVal(g->sp-1, &in);

	int type;
	if (in && out) type = BOTH;
	else if (in) type = IN;
	else type = OUT;

	if (listDevices(g, type)) return errNone;
	return errFailed;
}

int prAudioDevice_New(struct VMGlobals *g, int numArgsPushed)
{
	char inAudioDeviceNameChr[255];
	AudioDeviceID theAudioDeviceID = NULL;

	PyrSlot *receiver = g->sp - 1;
	PyrSlot *deviceNameSlot = g->sp;
	
	if ( slotStrVal(deviceNameSlot, inAudioDeviceNameChr, 255) ) return errFailed;
	
	UInt32 numDevices, i;

	unsigned long count;
	OSStatus err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &count, 0);
	AudioDeviceID *devices = (AudioDeviceID*)malloc(count);	
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &count, devices);
	if (err!=kAudioHardwareNoError)
	{
		post ("Error: AudioHardwareGetPropertyInfo( ) %ld - kAudioHardwarePropertyDevices\n", err);
		free(devices);
		return errNone;
	}
	
	numDevices = count / sizeof(AudioDeviceID);
	
	bool found = false;
	for (i=0; i<numDevices; i++)
	{
		if (!devices[i]) continue;
		
		err = AudioDeviceGetPropertyInfo(devices[i], 0, false, kAudioDevicePropertyDeviceName, &count, 0);
		if (err != kAudioHardwareNoError) 
		{
			post ("Error: AudioDeviceGetPropertyInfo( ) %ld - kAudioDevicePropertyDeviceName\n", err);
			break;
		}

		char *name = (char*)malloc(count);
		err = AudioDeviceGetProperty(devices[i], 0, false, kAudioDevicePropertyDeviceName, &count, name);
		if (err != kAudioHardwareNoError)
		{
			post ("Error: AudioDeviceGetProperty( ) %ld - kAudioDevicePropertyDeviceName\n", err);
			free(name);
			break;
		}
		
		if (!strcmp( inAudioDeviceNameChr, name )) {
			theAudioDeviceID = devices[i];
			free(name);
			found = true;
			break;
		};
		
		free(name);
	}
	
	if (!found) {
		post ("AudioDevice_New( ) Error: No '%s' AudioDevice was found.\n", inAudioDeviceNameChr);

		SetInt(slotRawObject(receiver)->slots + 0, -1);
		
		return errFailed;
	}

	SetInt(slotRawObject(receiver)->slots + 0, theAudioDeviceID);

	return errNone;
}

int prAudioDevice_GetSampleTime(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	AudioDeviceID iDevice = (AudioDeviceID)slotRawPtr(&slotRawObject(a)->slots[0]);
	if (!iDevice) return errNone;
	
	AudioTimeStamp	outTime;
	OSStatus err = AudioDeviceGetCurrentTime(iDevice, &outTime);
	if (err) {
		post ("AudioDeviceGetCurrentTime Error: %ld - is Server.internal running?\n", err);
		SetFloat(g->sp, 0.0);
		return errNone;
	}

	SetFloat(g->sp, outTime.mSampleTime);

	return errNone;
}

int prAudioDevice_GetHostTime(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	AudioDeviceID iDevice = (AudioDeviceID)slotRawPtr(&slotRawObject(a)->slots[0]);
	if (!iDevice) return errNone;
	
	AudioTimeStamp	outTime;
	OSStatus err = AudioDeviceGetCurrentTime(iDevice, &outTime);
	if (err) {
		post ("AudioDeviceGetCurrentTime Error: %ld - is Server.internal running?\n", err);
		SetFloat(g->sp, 0.0);
		return errNone;
	}

	if (outTime.mFlags && kAudioTimeStampHostTimeValid) {
		double audioHostTime = 1e-9 * (double)(AudioConvertHostTimeToNanos(outTime.mHostTime) /* - gHostStartNanos */ );
		SetFloat(g->sp, audioHostTime);
	} else {
		post ("AudioDeviceGetCurrentTime Error: result kAudioTimeStampHostTimeValid is not valid.\n", err);
		SetNil(g->sp);
	}
	
	return errNone;
}

int prAudioDevice_GetRateScalar(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	AudioDeviceID iDevice = (AudioDeviceID)slotRawPtr(&slotRawObject(a)->slots[0]);
	if (!iDevice) return errNone;

	AudioTimeStamp	outTime;
	OSStatus err = AudioDeviceGetCurrentTime(iDevice, &outTime);
	if (err) {
		post ("AudioDeviceGetCurrentTime Error: %ld - is Server.internal running?\n", err);
		SetFloat(g->sp, 1.0);
		return errNone;
	}

	SetFloat(g->sp, outTime.mRateScalar);

	return errNone;
}

void initCoreAudioPrimitives()
{
	int base, index;
	
	index = 0;
	base = nextPrimitiveIndex();

	definePrimitive(base, index++, "_ListAudioDevices", prListAudioDevices, 3, 0);
	definePrimitive(base, index++, "_AudioDevice_New", prAudioDevice_New, 2, 0);
	definePrimitive(base, index++, "_AudioDevice_GetSampleTime", prAudioDevice_GetSampleTime, 1, 0);
	definePrimitive(base, index++, "_AudioDevice_GetHostTime", prAudioDevice_GetHostTime, 1, 0);
	definePrimitive(base, index++, "_AudioDevice_GetRateScalar", prAudioDevice_GetRateScalar, 1, 0);
}
