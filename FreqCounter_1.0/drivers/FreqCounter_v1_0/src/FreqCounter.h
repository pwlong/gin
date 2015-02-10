
#ifndef FREQCOUNTER_H
#define FREQCOUNTER_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"

#define FREQCOUNTER_S00_AXI_SLV_REG0_OFFSET 0
#define FREQCOUNTER_S00_AXI_SLV_REG1_OFFSET 4
#define FREQCOUNTER_S00_AXI_SLV_REG2_OFFSET 8
#define FREQCOUNTER_S00_AXI_SLV_REG3_OFFSET 12


/**************************** Type Definitions *****************************/
/* These guys are the low-level drivers provided by Xilinx tool */

/**
 *
 * Write a value to a FREQCOUNTER register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the FREQCOUNTERdevice.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void FREQCOUNTER_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define FREQCOUNTER_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a FREQCOUNTER register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the FREQCOUNTER device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 FREQCOUNTER_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define FREQCOUNTER_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
	
/* FreqCounter register mappings */
typedef struct
{
	u32	highCountReg	FREQCOUNTER_S00_AXI_SLV_REG0_OFFSET;
	u32	lowCountReg		FREQCOUNTER_S00_AXI_SLV_REG1_OFFSET;
	u32 reserved0		FREQCOUNTER_S00_AXI_SLV_REG2_OFFSET;
	u32 reserved1		FREQCOUNTER_S00_AXI_SLV_REG3_OFFSET;	
} volatile _freqCounterReg, *FreqCounterReg;


/************************** Function Prototypes ****************************/
/**
 *
 * @name FREQCOUNTER_Reg_SelfTest
 *
 * @author	Xilinx tool
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 
 *
 * @param   baseaddr_p is the base address of the FREQCOUNTER instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus FREQCOUNTER_Reg_SelfTest(void * baseaddr_p);


/**
 *
 * @name	FREQCOUNTER_Initialize
 *
 * TODO: description here
 *
 * @author	Paul Long <paul@thelongs.ws>
 *
 * @date 	10 February 2015
 *
 * @param   BaseAddr is the base address of the Frequency Counter HW device
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 *
 */
XStatus FREQCOUNTER_Initialize(u32 BaseAddr);

/**
 *
 * @name	FREQCOUNTER_GetCounts
 *
 * TODO: description here
 *
 * @author	Paul Long <paul@thelongs.ws>
 *
 * @date 	10 February 2015
 *
 * @param   
 *
 * @return
 *
 *
 */
void FREQCOUNTER_Get Counts(u32 BaseAddr, u32* HighCount, u32* LowCount);


/**
 *
 * @name	FREQCOUNTER_CountsToVolts
 *
 * TODO: description here
 *
 * @author	Paul Long <paul@thelongs.ws>
 *
 * @date 	10 February 2015
 *
 * @param   
 *
 * @return
 *
 *
 */
u32 FREQCOUNTER_CountsToVolts(u32* HighCount, u32* LowCount);

/**
 *
 * @name	FREQCOUNTER_ScaleCounts
 *
 * TODO: description here
 *
 * @author	Paul Long <paul@thelongs.ws>
 *
 * @date 	10 February 2015
 *
 * @param   
 *
 * @return
 *
 *
 */
void FREQCOUNTER_ScaleCounts(u32* HighCount, u32* LowCount);






#endif // FREQCOUNTER_H
