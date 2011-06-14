ObjectWithArgs {
	
	var <args;
	
	// args are an array of key, value pairs: [ key, value, key, value ...etc ]
	
	keys { ^(args ? [])[0,2..] }
	argNames { ^this.keys }
	
	values { ^(args ? [])[1,3..] }
	values_ { |newValues|  
		var keys = this.keys;
		newValues[..keys.size-1].do({ |val, i|
			this.setArg( keys[i], val )
		});
	}

	setArg { |key, value|
		var index;
		index = this.keys.indexOf( key );
		if( index.notNil ) { 
			args[ (index * 2) + 1 ] = value;
			this.changed( key, value );
		} {
			"%:% arg % not found".format( this.class, thisMethod.name, key ).warn;
		};	
	}
	
	getArg { |key|
		var index;
		index = this.keys.indexOf( key );
		if( index.notNil ) { 
			^args[ (index * 2) + 1 ] 
		} { 
			"%:% arg % not found".format( this.class, thisMethod.name, key ).warn;
			^nil 
		};
	}
	
	at { |key| ^this.getArg( key ) }
	put { |key| ^this.setArg( key ) }

	doesNotUnderstand { |selector ...args| 
		// bypasses errors; warning if arg not found
		if( selector.isSetter ) { 
			this.setArg( selector.asGetter, *args ) 
		} {
			^this.getArg( selector );
		};	
	}

}

+ SequenceableCollection {
	
	pairsAt { |key| // could be optimized
		var index = this[0,2..].indexOf( key );
		if( index.notNil ) { 
			^this.at( (index * 2) + 1 );
		} {
			^nil 
		};
	}
	
	pairsPut { |key, value| // only puts if key is found
		var index = this[0,2..].indexOf( key );
		if( index.notNil ) { 
			this.put((index * 2) + 1, value);
		};
	}
}
