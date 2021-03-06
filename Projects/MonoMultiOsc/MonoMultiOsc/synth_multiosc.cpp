/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <Arduino.h>
#include "synth_multiosc.h"
#include "utility/dspinst.h"
#include "wavegen.h"

// bitPerOctaveLUTs.c
extern "C" {
extern const uint16_t twoPowers12bit[4096];
extern const float note2bpo[129];
}
// data_waveforms.c
extern "C" {
extern const int16_t AudioWaveformSine[257];
}


void AudioSynthMultiOsc::update(void)
{
	audio_block_t *block, *ampModBlock, *bpoABlock, *bpoBBlock, *bpoCBlock, *bpoDBlock, *centABlock, *centBBlock, *centCBlock, *centDBlock;
	uint32_t i, ph[4], inc, index, scale, input[4], ampInput, centInput[4];
	int32_t val1, val2[4];
	uint16_t inputFractional, inputWhole;
	uint32_t baseFreq, powerResult;
	int32_t sampleAcumulator;
	ampModBlock = receiveWritable(0);
	if (!ampModBlock) {
		release(ampModBlock);
		return;
	}
	bpoABlock = receiveWritable(1);
	if (!bpoABlock) {
		release(bpoABlock);
		return;
	}
	bpoBBlock = receiveWritable(2);
	if (!bpoBBlock) {
		release(bpoBBlock);
		return;
	}
	bpoCBlock = receiveWritable(3);
	if (!bpoCBlock) {
		release(bpoCBlock);
		//return;
	}
	bpoDBlock = receiveWritable(4);
	if (!bpoDBlock) {
		release(bpoDBlock);
		//return;
	}
	centABlock = receiveWritable(5);
	if (!centABlock) {
		release(centABlock);
		//return;
	}
	centBBlock = receiveWritable(6);
	if (!centBBlock) {
		release(centBBlock);
		//return;
	}
	centCBlock = receiveWritable(7);
	if (!centCBlock) {
		release(centCBlock);
		//return;
	}
	centDBlock = receiveWritable(8);
	if (!centDBlock) {
		release(centDBlock);
		//return;
	}
	block = allocate();
	if (block) {
		ph[0] = phase_accumulator[0];
		ph[1] = phase_accumulator[1];
		ph[2] = phase_accumulator[2];
		ph[3] = phase_accumulator[3];
		//inc = phase_increment;
		//Rather then this, calculate inc sample by sample
		
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i++)
		{
			sampleAcumulator = 0;
			input[0] = bpoABlock->data[i];  //Get incomming pitch data
			input[1] = bpoBBlock->data[i];  //Get incomming pitch data
			input[2] = bpoCBlock->data[i];  //Get incomming pitch data
			input[3] = bpoDBlock->data[i];  //Get incomming pitch data
			ampInput = ampModBlock->data[i] >> 2 ;
			centInput[0] = centABlock->data[i];
			centInput[1] = centBBlock->data[i];
			centInput[2] = centCBlock->data[i];
			centInput[3] = centDBlock->data[i];
			//for( int j = 0; j < 2; j++)
			//{
			//	//index = ph[j] >> 24;
			//	//val1[j] = waveFormPointerA[index];
			//	//val2[j] = waveFormPointerA[index+1];
			//	//scale = (ph[j] >> 8) & 0xFFFF;
			//	//val2[j] *= scale;
			//	//val1[j] *= 0x10000 - scale;
			//	
			//	//sampleAcumulator += waveFormPointerA[ph[j] >> 24];
			//	sampleAcumulator += multiply_32x32_rshift32(waveFormPointerA[ph[j] >> 24], ampInput );
			//	//sampleAcumulator += multiply_32x32_rshift32(waveFormPointerA[index], ampModBlock->data[i]);
			//	//sampleAcumulator += multiply_32x32_rshift32(val1[j] + val2[j], ampModBlock->data[i] >> 1);
			//	
			//	//do it the old way
			//	inputFractional = input[j] & 0x0FFF;  //Get only the RHS
			//	inputWhole = ( input[j] & 0x7000 ) >> 12;  //Get only the LHS
			//	
			//	baseFreq = 0x76D6A;  //This is a 24 bit number
			//	baseFreq = baseFreq << inputWhole;
			//	powerResult = ((uint64_t)(baseFreq >> 8) * twoPowers12bit[inputFractional]) >> 16;
			//	baseFreq = powerResult + (baseFreq >> 8);
			//	
			//	//need to format for correct inc.
			//	inc = baseFreq << 8;
			//	
			//	ph[j] += inc; //Store the new phase
			//}
				//First
				index = ph[0] >> 24;
				val1 = waveFormPointerA[index];
				val1 = val1 * staticAmp[0] / 256;
				sampleAcumulator += multiply_32x32_rshift32(val1 << 16, ampInput);
				//do it the old way
				inputFractional = input[0] & 0x0FFF;  //Get only the RHS
				inputWhole = ( input[0] & 0x7000 ) >> 12;  //Get only the LHS
				baseFreq = 0x76D6A;  //This is a 24 bit number
				baseFreq = baseFreq << inputWhole;
				powerResult = ((uint64_t)baseFreq * twoPowers12bit[inputFractional]) >> 24;
				baseFreq = powerResult + (baseFreq >> 8) + ((int32_t)centInput[0] >> 6);
				inc = baseFreq << 8;
				ph[0] += inc; //Store the new phase
				//second
				index = ph[1] >> 24;
				val1 = waveFormPointerA[index];
				val1 = val1 * staticAmp[1] / 256;
				sampleAcumulator += multiply_32x32_rshift32(val1 << 16, ampInput);
				//do it the old way
				inputFractional = input[1] & 0x0FFF;  //Get only the RHS
				inputWhole = ( input[1] & 0x7000 ) >> 12;  //Get only the LHS
				baseFreq = 0x76D6A;  //This is a 24 bit number
				baseFreq = baseFreq << inputWhole;
				powerResult = ((uint64_t)baseFreq * twoPowers12bit[inputFractional]) >> 24;
				baseFreq = powerResult + (baseFreq >> 8) + ((int32_t)centInput[1] >> 6);
				inc = baseFreq << 8;
				ph[1] += inc; //Store the new phase
				//Third
				index = ph[2] >> 24;
				val1 = waveFormPointerA[index];
				val1 = val1 * staticAmp[2] / 256;
				sampleAcumulator += multiply_32x32_rshift32(val1 << 16, ampInput);
				//do it the old way
				inputFractional = input[2] & 0x0FFF;  //Get only the RHS
				inputWhole = ( input[2] & 0x7000 ) >> 12;  //Get only the LHS
				baseFreq = 0x76D6A;  //This is a 24 bit number
				baseFreq = baseFreq << inputWhole;
				powerResult = ((uint64_t)baseFreq * twoPowers12bit[inputFractional]) >> 24;
				baseFreq = powerResult + (baseFreq >> 8) + ((int32_t)centInput[2] >> 6);
				inc = baseFreq << 8;
				ph[2] += inc; //Store the new phase
				//Fourth
				index = ph[3] >> 24;
				val1 = waveFormPointerA[index];
				val1 = val1 * staticAmp[3] / 256;
				sampleAcumulator += multiply_32x32_rshift32(val1 << 16, ampInput);
				//do it the old way
				inputFractional = input[3] & 0x0FFF;  //Get only the RHS
				inputWhole = ( input[3] & 0x7000 ) >> 12;  //Get only the LHS
				baseFreq = 0x76D6A;  //This is a 24 bit number
				baseFreq = baseFreq << inputWhole;
				powerResult = ((uint64_t)baseFreq * twoPowers12bit[inputFractional]) >> 24;
				baseFreq = powerResult + (baseFreq >> 8) + ((int32_t)centInput[3] >> 6);
				inc = baseFreq << 8;
				ph[3] += inc; //Store the new phase

				
			block->data[i] = sampleAcumulator >> 2;
		}
		phase_accumulator[0] = ph[0];
		phase_accumulator[1] = ph[1];
		phase_accumulator[2] = ph[2];
		phase_accumulator[3] = ph[3];
		transmit(block);
		release(block);
		release(ampModBlock);
		release(bpoABlock);
		release(bpoBBlock);
		release(bpoCBlock);
		release(bpoDBlock);
		release(centABlock);
		release(centBBlock);
		release(centCBlock);
		release(centDBlock);
		return;
	}
	
	//If there was no block to process, increase the phase accumulator by the number of skipped samples anyway.
	phase_accumulator[0] += phase_increment * AUDIO_BLOCK_SAMPLES;
	phase_accumulator[1] += phase_increment * AUDIO_BLOCK_SAMPLES;
	phase_accumulator[2] += phase_increment * AUDIO_BLOCK_SAMPLES;
	phase_accumulator[3] += phase_increment * AUDIO_BLOCK_SAMPLES;
}

void AudioSynthMultiOsc::begin(void)
{
    WaveGenerator testWave;
	waveFormPointerA = testWave.allocateU16_257();
	waveFormPointerB = testWave.allocateU16_257();
	Serial.println("During multiOSC.begin(),");
	uint32_t address;
	Serial.print("WFPA = 0x");
	address = (uint32_t)&waveFormPointerA[0];
	Serial.println(address, HEX);
	Serial.print("WFPB = 0x");
	address = (uint32_t)&waveFormPointerB[0];
	Serial.println(address, HEX);	
    testWave.setParameters( 255, 255, 0, 0, 45 );
    testWave.writeWaveU16_257( waveFormPointerA );	
}

int16_t * AudioSynthMultiOsc::getPointer( uint8_t oscNumber ) //Pass number 0 = OSC A, 1 = OSC B
{
	switch( oscNumber )
	{
		case 0:
		return waveFormPointerA;
		break;
		case 1:
		return waveFormPointerB;
		break;
		default:
		break;
	}
	return NULL;
}

void AudioSynthMultiOsc::setPointer( uint8_t oscNumber, int16_t * pointerVar ) //Pass number 0 = OSC A, 1 = OSC B
{
	switch( oscNumber )
	{
		case 0:
		waveFormPointerA = pointerVar;
		break;
		case 1:
		waveFormPointerB = pointerVar;
		break;
		default:
		break;
	}
}