#include "sdio.h"

/*
   pg. 968 commands are fixed with 48 bits
   pg. 969 responses can be 48 (short) or 136 (long) bits.

   pg. 972 - data xfer in FIFO of 32 words (32 bits per word) = 4 x 32 = 128
  bytes operates in APB2 clock domain (PCLK2)

   pg. 975 - example of read procedure using DMA

   pg. 978 card identification process
   1. bus is activiated
   2. SDIO card host broadcasts SD_APP_OP_COND (ACMD41)
   3. response is operation condition registers from all cards
   4. incompatible cards placed in inactive state
   5. SDIO host broadcasts ALL_SEND_CID (CMD2) to all active cards
   6. active cards send their CID numbers, enter identification state
   7. SDIO card host issues SET_RELATIVE_ADDR (CMD3) to that card.
   This new address is the RCA (relative card address); shorter than CID.
   8. SDIO card host repeats for other cards (if applicable) then waits for
  timeout

  pg. 979 Block read
  CMD17 (READ_SINGLE_BLOCK) initiates a block read and after completing the
  xfer, card returns to xfer state. CMD18 (READ_MULTIPLE_BLOCK) start a xfer of
  several consecutive blocks Host can abort at any time during xfer by sending a
  STOP transmission command.

  pg. 981 wide bus selection / deselection
  4-bit bus width is selected/deselected using SET_BUS_WIDTH (ACMD6)
  default bus width after power-up or GO_IDEL_STATE (CMD0) is 1 bit.
  ACMD6 is only valid in xfer state, ie can only be changed after a card is
  selected by SELECT/DESELECT_CARD (CMD7)

  Card status register pg. 986-987
  bits 12:9 are CURRENT_STATE idle, ready, stby, tran, data etc
  bit 8: READY_FOR_DATA

  pg. 993 'Application-specific commands' (ACMD)
  when card receives APP_CMD (CMD55), it interprets subsequent cmd as an ACMD.
  Differentiates the command number from the 'general commands' (CMD).

  pg. 996 Response formats
  R1 (normal response) = 48 bits
  R2 (CID, CSD register) = 136 bits
  R3 (OCR register) = 48 bits
  R4 (Fast I/O) = 48 bits

  pg. 1002 SDIO register definitions (finally)

  !! pg. 1003, when in identification mode, SDIO_CK freq must be less than 400
  khz once relative card addresses are assigned, can increase clock speed using
  SDIO_CLKCR (sdio clock control register)

  pg. 1004, SDIO_ARG register.
  if command requires an arg, must write arg here first before writing cmd to
  cmd register. SDIO_CMD register -> command type bits control command path
  state machine CPSM.
  !!! after data write, cannot write to register again for 3 SDIOCLK clock
  periods plues 2 PCLK2 clock periods. SD cards can only send short repsonses
  (multimedia can be either short or long).

  pg. 1007 SDIO_DLEN is number of data bytes to be xfered.
  pg. 1008 SDIO_DCTL
  - bits 7:4 define data block size (1001 is for 512 bytes).
  contains DMAEN bit.
  - DTDIR sets direction card -> controller or controller -> card
  - DTEN bit enables the xfer to start
  - after a write, must wait 3 SDIOCLK (48 mhz) clock periods plues 2 PCKL2
  periods

  pg. 1009 SDIO_STA (status register)
  - read only
  - CMDSENT, CMDREND
  - bits about when FIFO is full, when data rx or tx is in progress

  pg. 1012 SDIO_MASK register configures interrupts
  pg. 1015 SDIO_FIFO registers, 32 sequential addresses of 32 bit wide
  registers. pg. 1016 SDIO register map
*/
