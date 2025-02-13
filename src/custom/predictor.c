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

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// Add your own Branch Predictor data structures here
//
uint32_t *choiceTable; // choosing between global vs local table
uint32_t *globalTable; // global counter table
uint32_t *localTable; // local counter table
uint32_t *localBranchTable; // local branch history table

uint32_t gHistory;
uint32_t pcbits;

uint32_t pcMask;
uint32_t gHistMask;
uint32_t lHistMask;


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

  gHistMask = 0;
  lHistMask = 0;
  pcMask = 0;

  ghistoryBits = 13;
  lhistoryBits = 3;
  pcIndexBits = 13;

  switch(bpType) {
    case CUSTOM:

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
        gHistMask = gHistMask << 1 | 1;
      }

      for(int i = 0; i < lhistoryBits; i++) {
        lHistMask = lHistMask << 1 | 1;
      }

      for(int i = 0; i < pcIndexBits; i++) {
        pcMask = pcMask << 1 | 1;
      }
    default: return;
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
    case CUSTOM:
      // use masks to get important bits
      pcBits = pcMask & pc;
      ghistBits = gHistMask & gHistory;
      lhistBits = lHistMask & localBranchTable[pcBits];

      // xor to get gshare index
      int16_t gshareIdx = pcBits ^ ghistBits;

      // find choice
      uint32_t choice = choiceTable[gshareIdx];
      uint32_t pred;

      // if choice favors global history
      if (choice == WN){
        pred = localTable[lhistBits];
      } else {
        pred = globalTable[gshareIdx];
      }

      // pred = globalTable[gshareIdx];

      // return TAKEN if MSB of 2-bit counter is 1
      if(pred == ST || pred == WT)
        return TAKEN;
      
      return NOTTAKEN;

    default: break;
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
  uint32_t lhistBits;

  switch (bpType) {
    case CUSTOM:
      // use masks to get important bits
      pcBits = pcMask & pc;
      ghistBits = gHistMask & gHistory;
      lhistBits = lHistMask & localBranchTable[pcBits];

      // xor to get gshare index
      int16_t gshareIdx = pcBits ^ ghistBits;

      // find choice and predictions
      uint32_t choice = choiceTable[gshareIdx];
      uint32_t globalPred;
      uint32_t localPred;
      
      globalPred = globalTable[gshareIdx];
      localPred = localTable[lhistBits];

      if(outcome == globalPred && outcome != localPred) {
        // if choice isn't already max-ed at 3
        if(choice != ST)
          choiceTable[gshareIdx] += 1; 
      } else if(outcome != globalPred && outcome == localPred) {
        // if choice isn't already min-ed at 0
        if(choice != SN)
          choiceTable[gshareIdx] -= 1;
      }

      // update counter tables
      if(outcome == TAKEN) {
        if(globalPred != ST)
          globalTable[gshareIdx] += 1;

        if(localPred != ST)
          localTable[lhistBits] += 1;
      } else {
        if(globalPred != SN)
          globalTable[gshareIdx] -= 1;
        
        if(localPred != SN)
          localTable[lhistBits] -= 1;
      }

      // update global history and local branch table
      gHistory = (gHistory << 1) | outcome;
      gHistory = gHistMask & gHistory;

      localBranchTable[lhistBits] = (localBranchTable[lhistBits] << 1) | outcome;
      localBranchTable[lhistBits] = lHistMask & localBranchTable[lhistBits];

      return;

    default: return;
  }
}
