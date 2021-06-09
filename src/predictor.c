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
// TODO:Student Information
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

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// Add your own Branch Predictor data structures here
//
uint32_t *table; // gshare counter table

uint32_t *choiceTable; // table to decide local vs global
uint32_t *globalTable; // table for global 2-bit counters
uint32_t *localTable; // table for local 2-bit counters
uint32_t *localBranchTable; // local branch history table

uint32_t gHistory; // global history of branch outcomes

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

  switch(bpType) {
    case GSHARE:

      size = ghistoryBits << 1; // size = 2 * ghistoryBits
      table = (uint32_t*) calloc(size, sizeof(uint32_t)); // init table with zeros

      // init masks
      for(int i = 0; i < ghistoryBits; i++) {
        ghistmask = ghistmask << 1 | 1; // left shift and set LSB to 1
        pcmask = pcmask << 1 | 1; // gshare pc and ghist bits are equal length
      }

    case TOURNAMENT:

      // init tables
      size = ghistoryBits << 1; // size = (bits for counter = 2) * ghistoryBits
      globalTable = (uint32_t*) calloc(size, sizeof(uint32_t));
      choiceTable = (uint32_t*) calloc(size, sizeof(uint32_t));

      size = lhistoryBits << 1; // size = (bits for counter = 2) * lhistoryBits
      localTable = (uint32_t*) calloc(size, sizeof(uint32_t));

      size = pcIndexBits << 1;
      localBranchTable = (uint32_t*) calloc(size, sizeof(uint32_t));

      // init choice table to weakly favor global history
      for(int i = 0; i < ghistoryBits; i++) {
        choiceTable[i] = WT;
      }

      // init masks
      for(int i = 0; i < ghistoryBits; i++) {
        ghistmask = ghistmask << 1 | 1;
      }

      for(int i = 0; i < lhistoryBits; i++) {
        lhistmask = lhistmask << 1 | 1;
      }

      for(int i = 0; i < pcIndexBits; i++) {
        pcmask = pcmask << 1 | 1;
      }

    case CUSTOM:
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
    case STATIC:
      return TAKEN;
    case GSHARE:
      // use masks to get important bits
      pcBits = pcmask & pc;
      ghistBits = ghistmask & gHistory;
      
      // xor to get gshare index
      int16_t gshareIdx = pcBits ^ ghistBits;

      // get prediction from gshare table
      uint32_t pred;
      pred = table[gshareIdx];

      // return TAKEN if MSB of 2-bit counter is 1
      if(pred == ST || pred == WT) {
        return TAKEN;
      }

      return NOTTAKEN;

    case TOURNAMENT:
      // use masks to get important bits
      pcBits = pcmask & pc;
      ghistBits = ghistmask & ghistory;
      lhistBits = lhistmask & localBranchTable[pcBits];

      // find choice
      choice = choiceTable[ghistBits];
      uint32_t pred;

      // if choice favors global history
      if(choice == ST || choice == WT) {
        pred = globalTable[ghistBits];
      } else {
        pred = localTable[lhistBits];
      }

      // return TAKEN if MSB of 2-bit counter is 1
      if(pred == ST || pred == WT)
        return TAKEN;
      
      return NOTTAKEN;

    case CUSTOM:
    default:
      break;
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
    case GSHARE:
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

    case TOURNAMENT:
      // use masks to get important bits
      pcBits = pcmask & pc;
      ghistBits = ghistmask & ghistory;
      lhistBits = lhistmask & localBranchTable[pcBits];

      // find choice and predictions
      choice = choiceTable[ghistBits];
      uint32_t globalPred;
      uint32_t localPred;
      
      globalPred = globalTable[ghistBits];
      localPred = localTable[lhistBits];

      if(outcome == globalPred && outcome != localPred) {
        // if choice isn't already max-ed at 3
        if(choice != ST)
          choiceTable[gHistBits] += 1; 
      } else if(outcome != globalPred && outcome == localPred) {
        // if choice isn't already min-ed at 0
        if(choice != SN)
          choiceTable[gHistBits] -= 1;
      }

      // update counter tables
      if(outcome == TAKEN) {
        if(globalPred != ST)
          globalTable[ghistBits] += 1

        if(localPred != ST)
          localTable[locidx] += 1;
      } else {
        if(globalPred != SN)
          globalTable[ghistBits] -= 1;
        
        if(localPred != SN)
          localTable[locidx] -= 1;
      }

      // update global history and local branch table
      gHistory = (gHistory << 1) | outcome;
      gHistory = ghistmask & gHistory;

      localBranchTable[locidx] = (localBranchTable[locidx] << 1) | outcome;
      localBranchTable[locidx] = lhistmask & localBranchTable[locidx];

      return;

    case CUSTOM:
    default:
      return;
  }
}
