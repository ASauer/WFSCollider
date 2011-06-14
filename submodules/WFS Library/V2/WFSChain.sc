/*
// example

(
// create two WFSUnitDefs

WFSUnitDef( \sine, { |freq = 440, amp = 0.1| 
	Out.ar( 0, SinOsc.ar( freq, 0, amp ) ) 
} ).loadSynthDef;

WFSUnitDef( \vibrato, { |rate = 1, amount = 1.0| 
	Out.ar( 0, SinOsc.ar( rate ).range(1-amount,1) * In.ar( 0 ) ) 
} ).loadSynthDef;
)

x = WFSChain( \sine, \vibrato );

x.start;

x.stop;

(

// a styled gui in user-defined window
// -- to be replaced by WFSChainGUI later -- 

w = Window( "x", Rect( 300,25,200,200 ) ).front;
w.addFlowLayout;
RoundView.useWithSkin( ( 
	labelWidth: 40, 
	font: Font( Font.defaultSansFace, 10 ), 
	hiliteColor: Color.gray(0.33),
), { 
	SmoothButton( w, 16@16 )
		.label_( ['power', 'power'] )
		.hiliteColor_( Color.green.alpha_(0.5) )
		.action_( [ { x.start }, { x.stop } ] )
		.value_( (x.groups.size > 0).binaryValue );
	y = x.units.collect({ |item|
		StaticText( w, (w.view.bounds.width - 8)@16 )
			.string_( " " ++ item.defName.asString )
			.font_( RoundView.skin.font.boldVariant )
			.background_( Color.gray(0.8) );
		item.gui( w );
	}); 
});

)

*/

WFSChain {
	var <>units, <>groups;
	
	*new { |...units|
		^super.newCopyArgs( units.collect(_.asWFSUnit) )
	}
	
	start { |server|
		var group;
		group = Group( server )
				.startAction_({ |synth|
					groups = groups.add( group ); 
						// only add if started (in case this is a bundle)
					this.changed( \go, group ); 
				})
				.freeAction_({ |synth| 
					groups.remove( group ); 
					this.changed( \end, group ); 
				});
		this.changed( \start, group );
		units.do( _.start(group) );
		^group;
	}
	
	free { groups.do(_.free) }
	stop { this.free }
	
	resetGroups { groups = []; } // after unexpected server quit
}