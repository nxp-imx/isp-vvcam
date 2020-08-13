#ifndef _VSI_CORE_GEN6_H
#define _VSI_CORE_GEN6_H


#define REG_TPG0      0x300
#define REG_TPG1      0x310
#define REG_DWE_CTRL  0x250
#define REG_VSE_CTRL  0x254



 int gen6_write_reg(void* dev,unsigned int addr,unsigned int val);
 int gen6_read_reg(void* dev,unsigned int addr,unsigned int *val);

#endif
