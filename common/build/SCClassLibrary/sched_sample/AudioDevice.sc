
// blackrain 2009

AudioDevice {
	var <deviceID;
	
	*new { arg device;
		^super.new.init(device);
	}
	init { arg device;
		this.prInit(device);
	}
	prInit { arg device;
		_AudioDevice_New
		^this.primitiveFailed;
	}
	sampleTime {
		_AudioDevice_GetSampleTime
		^this.primitiveFailed;
	}
	hostTime {
		_AudioDevice_GetHostTime
		^this.primitiveFailed;
	}
	rateScalar {
		_AudioDevice_GetRateScalar
		^this.primitiveFailed;
	}
	
}
