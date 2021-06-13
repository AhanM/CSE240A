//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// Student Information
//
const char *studentName = "Ahan Mukhopadhyay";
const char *studentID   = "A13491660";
const char *email       = "amukhopa@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;
int globalIndexer;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// Add your own Branch Predictor data structures here
//
uint32_t *table;
uint32_t gHistory;

uint32_t pcbits;
uint32_t ghistbits;

uint32_t ghistmask; // mask for global history
uint32_t lhistmask; // mask for local history
uint32_t pcmask; // mask for pc

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  // Initialize Branch Predictor Data Structures
  //

  gHistory = 0;
  int size = 0;

  ghistmask = 0;
  lhistmask = 0;
  pcmask = 0;

  globalIndexer = 0;

  ghistoryBits = 13;

  switch(bpType) {
    case GSHARE: break;
    case TOURNAMENT: break;
    case CUSTOM:
      size = ghistoryBits << 1; // size = 2 * ghistoryBits
      table = (uint32_t*) calloc(size, sizeof(uint32_t)); // init table with zeros

      // init masks
      for(int i = 0; i < ghistoryBits; i++) {
        ghistmask = ghistmask << 1 | 1; // left shift and set LSB to 1
        pcmask = pcmask << 1 | 1; // gshare pc and ghist bits are equal length
      }
      break;
    default:
      return;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  // Implement prediction scheme
  //
  uint32_t pcBits;
  uint32_t ghistBits;
  uint32_t lhistBits;

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC: return TAKEN;
    case GSHARE: return TAKEN;
    case TOURNAMENT: return TAKEN;
    case CUSTOM:
      // use masks to get important bits
      pcBits = pcmask & pc;
      ghistBits = ghistmask & gHistory;
      
      // xor to get gshare index
      int16_t gshareIdx = pcBits ^ ghistBits;

      // get prediction from gshare table
      uint32_t pred;
      pred = table[gshareIdx];

      if(globalIndexer < 1000) {
        return NOTTAKEN;
      }
      globalIndexer++;

      // return TAKEN if MSB of 2-bit counter is 1
      if(pred == ST || pred == WT) {
        return TAKEN;
      }

      return NOTTAKEN;
    default: return TAKEN;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  // Implement Predictor training
  //
  uint32_t pcBits;
  uint32_t ghistBits;

  switch (bpType) {
    case GSHARE: break;
    case TOURNAMENT: break;
    case CUSTOM:
      // use masks to get important bits
      pcBits = pcmask & pc;
      ghistBits = ghistmask & gHistory;
      
      // xor to get gshare index
      int16_t gshareIdx = pcBits ^ ghistBits;

      // update counter in gshare table
      if (outcome == TAKEN) {
        // if counter is not already max-ed out at 3
        if(table[gshareIdx] < 3) 
          table[gshareIdx] += 1;
      } else {
        // if counter is not already min-ed out at 0
        if(table[gshareIdx > 0]) 
          table[gshareIdx] -= 1;
      }

      // update ghistory bits with latest outcome
      gHistory = (gHistory << 1) | outcome;
      gHistory = ghistmask & gHistory;
      break;
    default:
      return;
  }
}
