#define ICCM_BASE       0xA0000000
#define DCCM_BASE       0x00100000
#define UART_BASE       0xC0000000
#define SSI_BASE        0xC0001000
#define CPU_REGS_BASE   0xC0002000
#define GEMAC_REGS_BASE 0xC0003000
#define DDR_BASE        0x80000000
#define ERROR_BASE      0x30000000
  
#define REG_CPU_GPIO_IN   (* ( (volatile int*)(CPU_REGS_BASE+0x00000000 ) ) )
#define REG_CPU_GPIO_OUT  (* ( (volatile int*)(CPU_REGS_BASE+0x00000004 ) ) )
#define REG_CPU_GPIO_OEN  (* ( (volatile int*)(CPU_REGS_BASE+0x00000008 ) ) )
#define REG_CPU_RST_CTL   (* ( (volatile int*)(CPU_REGS_BASE+0x00000014 ) ) )
#define REG_CPU_HALT_CTL  (* ( (volatile int*)(CPU_REGS_BASE+0x00000018 ) ) )
#define REG_CPU_SSI_MASK  (* ( (volatile int*)(CPU_REGS_BASE+0x00000040 ) ) )
#define REG_CPU_DDRC_CFG  (* ( (volatile int*)(CPU_REGS_BASE+0x00000080 ) ) )
#define REG_CPU_DDRC_STS  (* ( (volatile int*)(CPU_REGS_BASE+0x00000084 ) ) )
#define REG_CPU_HW_VER    (* ( (volatile int*)(CPU_REGS_BASE+0x000003fC ) ) )

#define REG_GEMAC_TX_CTL     (* ( (volatile int*)(GEMAC_REGS_BASE+0x00000000 ) ) )
#define REG_GEMAC_TXBUF_STS  (* ( (volatile int*)(GEMAC_REGS_BASE+0x00000004 ) ) )
#define REG_GEMAC_TXBUF_DATA (* ( (volatile int*)(GEMAC_REGS_BASE+0x00000008 ) ) )
#define REG_GEMAC_RX_CTL     (* ( (volatile int*)(GEMAC_REGS_BASE+0x00000010 ) ) )
#define REG_GEMAC_RXBUF_STS  (* ( (volatile int*)(GEMAC_REGS_BASE+0x00000014 ) ) )
#define REG_GEMAC_RXBUF_DATA (* ( (volatile int*)(GEMAC_REGS_BASE+0x00000018 ) ) )
#define REG_GEMAC_MAC_CFG    (* ( (volatile int*)(GEMAC_REGS_BASE+0x00000040 ) ) )

//SSI
#define REG_SSI_CTRLR0 		      (* ( (volatile int*)(SSI_BASE+0x00000000) ) )
#define REG_SSI_CTRLR1 		      (* ( (volatile int*)(SSI_BASE+0x00000004) ) )
#define REG_SSI_SSIENR 		      (* ( (volatile int*)(SSI_BASE+0x00000008) ) )
#define REG_SSI_MWCR 		      (* ( (volatile int*)(SSI_BASE+0x0000000C) ) )
#define REG_SSI_SER 		         (* ( (volatile int*)(SSI_BASE+0x00000010) ) )
#define REG_SSI_BAUDR 		      (* ( (volatile int*)(SSI_BASE+0x00000014) ) )
#define REG_SSI_TXFTLR 		      (* ( (volatile int*)(SSI_BASE+0x00000018) ) )
#define REG_SSI_RXFTLR 		      (* ( (volatile int*)(SSI_BASE+0x0000001C) ) )
#define REG_SSI_TXFLR 		      (* ( (volatile int*)(SSI_BASE+0x00000020) ) )
#define REG_SSI_RXFLR 		      (* ( (volatile int*)(SSI_BASE+0x00000024) ) )
#define REG_SSI_SR 		         (* ( (volatile int*)(SSI_BASE+0x00000028) ) )
#define REG_SSI_IMR 		         (* ( (volatile int*)(SSI_BASE+0x0000002C) ) )
#define REG_SSI_ISR 		         (* ( (volatile int*)(SSI_BASE+0x00000030) ) )
#define REG_SSI_RISR 		      (* ( (volatile int*)(SSI_BASE+0x00000034) ) )
#define REG_SSI_TXOICR 		      (* ( (volatile int*)(SSI_BASE+0x00000038) ) )
#define REG_SSI_RXOICR 		      (* ( (volatile int*)(SSI_BASE+0x0000003C) ) )
#define REG_SSI_RXUICR 		      (* ( (volatile int*)(SSI_BASE+0x00000040) ) )
#define REG_SSI_MSTICR 		      (* ( (volatile int*)(SSI_BASE+0x00000044) ) )
#define REG_SSI_ICR 		         (* ( (volatile int*)(SSI_BASE+0x00000048) ) )
#define REG_SSI_DMACR 		      (* ( (volatile int*)(SSI_BASE+0x0000004C) ) )
#define REG_SSI_DMATDLR 		   (* ( (volatile int*)(SSI_BASE+0x00000050) ) )
#define REG_SSI_DMARDLR 		   (* ( (volatile int*)(SSI_BASE+0x00000054) ) )
#define REG_SSI_IDR 		         (* ( (volatile int*)(SSI_BASE+0x00000058) ) )
#define REG_SSI_SSI_COMP_VERSION (* ( (volatile int*)(SSI_BASE+0x0000005C) ) )
#define REG_SSI_DR 		         (* ( (volatile int*)(SSI_BASE+0x00000060) ) )
#define REG_SSI_RX_SAMPLE_DLY    (* ( (volatile int*)(SSI_BASE+0x000000f0) ) )
#define REG_SSI_RSVD_0 		      (* ( (volatile int*)(SSI_BASE+0x000000f4) ) )
#define REG_SSI_RSVD_1 		      (* ( (volatile int*)(SSI_BASE+0x000000f8) ) )
#define REG_SSI_RSVD_2 		      (* ( (volatile int*)(SSI_BASE+0x000000fc) ) )

#define CS_FLASH_OVER        0x01

//UART
#define REG_UART_RBR             (* ( (volatile int*)(UART_BASE+0x00000000) ) )
#define REG_UART_THR             (* ( (volatile int*)(UART_BASE+0x00000000) ) )
#define REG_UART_DLL             (* ( (volatile int*)(UART_BASE+0x00000000) ) )
#define REG_UART_DLH             (* ( (volatile int*)(UART_BASE+0x00000004) ) )
#define REG_UART_IER             (* ( (volatile int*)(UART_BASE+0x00000004) ) )
#define REG_UART_IIR             (* ( (volatile int*)(UART_BASE+0x00000008) ) )
#define REG_UART_FCR             (* ( (volatile int*)(UART_BASE+0x00000008) ) )
#define REG_UART_LCR             (* ( (volatile int*)(UART_BASE+0x0000000C) ) )
#define REG_UART_MCR             (* ( (volatile int*)(UART_BASE+0x00000010) ) )
#define REG_UART_LSR             (* ( (volatile int*)(UART_BASE+0x00000014) ) )
#define REG_UART_MSR             (* ( (volatile int*)(UART_BASE+0x00000018) ) )
#define REG_UART_SCR             (* ( (volatile int*)(UART_BASE+0x0000001C) ) )
#define REG_UART_LPDLL           (* ( (volatile int*)(UART_BASE+0x00000020) ) )
#define REG_UART_LPDLH           (* ( (volatile int*)(UART_BASE+0x00000024) ) )
#define REG_UART_USR             (* ( (volatile int*)(UART_BASE+0x0000007C) ) )
#define REG_UART_TFL             (* ( (volatile int*)(UART_BASE+0x00000080) ) )
#define REG_UART_RFL             (* ( (volatile int*)(UART_BASE+0x00000084) ) )
#define REG_UART_SRR             (* ( (volatile int*)(UART_BASE+0x00000088) ) )
#define REG_UART_SRTS            (* ( (volatile int*)(UART_BASE+0x0000008C) ) )
#define REG_UART_SBCR            (* ( (volatile int*)(UART_BASE+0x00000090) ) )
#define REG_UART_SDMAM           (* ( (volatile int*)(UART_BASE+0x00000094) ) )
#define REG_UART_SFE             (* ( (volatile int*)(UART_BASE+0x00000098) ) )
#define REG_UART_SRT             (* ( (volatile int*)(UART_BASE+0x0000009C) ) )
#define REG_UART_STET            (* ( (volatile int*)(UART_BASE+0x000000A0) ) )
#define REG_UART_HTX             (* ( (volatile int*)(UART_BASE+0x000000A4) ) )
#define REG_UART_CPR             (* ( (volatile int*)(UART_BASE+0x000000F4) ) )
#define REG_UART_UCV             (* ( (volatile int*)(UART_BASE+0x000000F8) ) )
#define REG_UART_CTR             (* ( (volatile int*)(UART_BASE+0x000000FC) ) )

// DDR
void write_ddr (int,int);
int read_ddr (int);

