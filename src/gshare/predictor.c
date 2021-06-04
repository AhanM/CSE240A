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
uint32_t *table;
uint32_t ghistory;
uint32_t bitsmask;

uint32_t pcbits;
uint32_t ghistbits;

void printBinaryValue(unsigned int num)
{
    if(!num) return;

    printBinaryValue(num>>1);
    putchar((((num&1) == 1) ? '1' : '0'));
}

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  ghistory = 0;
  pcbits = 0;
  ghistbits = 0;

  for (int i = 0; i < ghistoryBits; i++) {
    bitsmask = bitsmask << 1 | 1;
  }
  
  // initialize global history table
  int size = 1 << ghistoryBits; // size = history bits * 2 bits per table entry

  switch(bpType) {
    case GSHARE:
      // calloc initializes table with zeros
      table = (uint32_t*) calloc(size, sizeof(uint32_t));
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

      // get lower bits for pc and g history
      pcbits = pc & bitsmask;
      ghistbits = ghistory & bitsmask;

      // compute table index and get prediction
      int16_t tableidx = pcbits ^ ghistbits;
      // setvbuf(stdout, NULL, _IONBF, 0);
      // printf("tableidx = %d\n", tableidx);
      // printf("pcbits = %x\n", pcbits);
      // printf("ghistbits = %x\n", ghistbits);

      uint32_t pred = table[tableidx];

      if(pred > 1)
        return TAKEN;
      else
        return NOTTAKEN;

    case TOURNAMENT:
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
  pcbits = 0;
  ghistbits = 0;

  switch(bpType) {
    case GSHARE:
    
      // compute index
      pcbits = pc & bitsmask;
      ghistbits = ghistory & bitsmask;
      int16_t tableidx = pcbits ^ ghistbits;

      // setvbuf(stdout, NULL, _IONBF, 0);
      // printf("tableidx = %d\n", tableidx);
      // printBinaryValue(pc);
      // printf("\n");
      // printBinaryValue(bitsmask);
      // printf("\n");
      // printBinaryValue(pcbits);
      // printf("\n");

      // tableidx = 0;
      int gbit = 0;

      // update table counter
      if(outcome == TAKEN) {
        if(table[tableidx]<3) table[tableidx]++;
        gbit = 1;
      } else {
        if(table[tableidx]>0) table[tableidx]--;
      }

      // update ghistory
      ghistory = ghistory << 1 | gbit;
    
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }
}
