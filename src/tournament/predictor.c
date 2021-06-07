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
uint32_t *gHistoryTable; // global counter table
uint32_t *lHistoryTable; // local counter table
uint32_t *localBHT; // local branch history table

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
  pcbits = 0;
  int size;

  switch(bpType) {
    case TOURNAMENT:
      // calloc initializes table with zeros
      size = 1 << ghistoryBits;
      gHistoryTable = (uint32_t*) calloc(size, sizeof(uint32_t));
      choiceTable = (uint32_t*) calloc(size, sizeof(uint32_t));

      size = 1 << lhistoryBits;
      lHistoryTable = (uint32_t*) calloc(size, sizeof(uint32_t));

      size = 1 << pcIndexBits;
      localBHT = (uint32_t*) calloc(size, sizeof(uint32_t));
      
      // initialize choice table to weakly favor gHistory
      for(int i = 0; i < size; i++) {
        choiceTable[i] = 2;
      }

      // initialize masks
      gHistMask = 0;
      for (int i = 0; i < ghistoryBits; i++) {
        gHistMask = gHistMask << 1 | 1;
      }

      lHistMask = 0;
      for (int i = 0; i < lhistoryBits; i++) {
        lHistMask = lHistMask << 1 | 1;
      }

      pcMask = 0;
      for (int i = 0; i < pcIndexBits; i++) {
        pcMask = pcMask << 1 | 1;
      }
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

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
    case TOURNAMENT:
      // insert code here 
      ghistoryBits = gHistMask & gHistory;
      uint32_t choice = choiceTable[ghistoryBits];

      uint32_t idx = pcMask & pc;
      uint32_t pred;

      if(choice<2) {
        uint32_t locHist = lHistMask & lHistoryTable[idx];
        pred = lHistoryTable[locHist];
      } else {
        pred = gHistoryTable[ghistoryBits];
      }

      if(pred > 1)
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

  switch(bpType) {
    case TOURNAMENT:

      ghistoryBits = gHistMask & gHistory;
      uint32_t choice = choiceTable[ghistoryBits];

      uint32_t pcidx = pcMask & pc;
      uint32_t locidx = lHistMask & localBHT[pcidx];

      // get local and global predictions
      uint32_t localVal = lHistoryTable[locidx];
      uint32_t globalVal = gHistoryTable[ghistoryBits];

      uint32_t localPred;
      uint32_t globalPred;

      if(localVal > 1) {
        localPred = TAKEN;
      } else {
        localPred = NOTTAKEN;
      }

      if(globalVal > 1) {
        globalPred = TAKEN;
      } else {
        localPred = NOTTAKEN;
      }

      if(localPred == outcome && globalPred != outcome && choice != 3) {
        choiceTable[ghistoryBits] += 1;
      } else if(localPred != outcome && globalPred == outcome && choice != 0) {
        choiceTable[ghistoryBits] -= 1;
      }

      // update global and local history counter tables
      if(outcome == TAKEN) {
        if(localVal < 3) {
          lHistoryTable[locidx] += 1;
        }

        if(globalVal < 3) {
          gHistoryTable[ghistoryBits] += 1;
        }

      } else {
        if(localVal > 0) {
          lHistoryTable[locidx] -= 1;
        }

        if(globalVal > 0) {
          gHistoryTable[ghistoryBits] -= 1;
        }
      }

      // update global history var and local branch history
      gHistory =  gHistMask & ((gHistory << 1) | outcome);
      localBHT[pcidx] = lHistMask & ((localBHT[pcidx] << 1) | outcome);

      return;

    case GSHARE:
      return;
    case CUSTOM:
    default:
      break;
  }
}
