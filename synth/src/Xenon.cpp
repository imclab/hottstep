#include "Xenon.h"
#include <math.h>
#include <iostream>

using namespace std;

namespace hottstep {
namespace synth {

class Xenon;

Xenon::Xenon( float tempo, float sampleRate, int bars, int divisionsInBar )
      : SequencePlayer( tempo, sampleRate, bars, divisionsInBar ),
        noise(2),
        lpfilter()
{
   envfilter.setupLowpass( sampleRate, 1000, 1 );
   lpfilter.setupLowpass( sampleRate, 2500 , 1.0f );
   setAdsr( 10, 10, .6, 100 );
}
void Xenon::reset() {
   pos = 0;
   lpfilter.reset();
   envfilter.reset();
   envFollow = 0;
}

float *Xenon::tick() {
   int note = getNotes()[ pos / lengthPerDivision ] ;
   int placeInNote = pos % lengthPerDivision;
   float ret;
   float gain = 0;
   if( note < 0 || note > 127 ) {
      ret = 0;
   } else {
      gain = sustainLevel;
      float n = 0;
      if( placeInNote < attackSamples ) {
         gain = placeInNote / (float) attackSamples ;
         n = noise.generateUniformRandomDeviate() * gain ;
      } else if( placeInNote < attackSamples + decaySamples ) {
         int p = placeInNote - attackSamples;
         float r = p / (float) decaySamples ;
         gain = ( 1 - r ) + sustainLevel * r ;
         n = noise.generateUniformRandomDeviate() * (1-r) ;
      } else if( placeInNote > lengthPerDivision - releaseSamples ) {
         int p = placeInNote - ( lengthPerDivision - releaseSamples ) ;
         float r = p / (float) releaseSamples ;
         gain = ( 1 - r ) * sustainLevel ;
      } else {
      }
      //cout << placeInNote << " : " << gain << " : " << sustainLevel << " : " << attackSamples ;
      //for( int i=0; i<80*gain; ++i )
      //   cout << "*" ;
      //cout << endl;
      ret = fabsf( sin( pitchTable[note] * placeInNote * 2 * M_PI / sampleRate ) );
      ret = ( ret - .5 ) * 2 ;

      //ret += n / 20;
      ret *= gain;
      ret *= 6 ;
      ret = clip( ret, .33 );
   }
   //cout << pos << " : " << ret << endl;
   ++pos;
   if( pos >= totalLength )
      pos = 0;

   //filter
   //cout << "-------1 " << ret << endl;
   ret = lpfilter.process( ret );
   //cout << "-------2 " << ret << endl;
   //cout << gain << " " << envFollow << " " << sampleRate << endl;
   envFollow = gain * .0005 + envFollow * .9995 ;
   envfilter.setupLowpass( sampleRate, 100 + envFollow * 700, 8 );
   ret = envfilter.process( ret );
   //cout << "-------3 " << ret << endl;
   ret = clip( ret, .95 );
   //cout << "-------4 " << ret << endl;
   //if( isnan( ret ) )
   //   exit(0);
   retVals[0] = ret;
   retVals[1] = ret;
   return retVals;
}

} //namespace synth
} //namepsace hotstep
