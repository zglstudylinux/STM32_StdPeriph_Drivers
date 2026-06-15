/**
 * @file    startup_stm32f10x_md.s
 * @brief   STM32F103C8T6 ARM GCC启动文件
 * @details 包含中断向量表、Reset_Handler、Default_Handler
 *          初始化数据段和BSS段，跳转到main函数
 * 
 * @note    本文件适用于STM32F103中等密度设备(MD)
 *          Flash: 64KB, RAM: 20KB
 */

/* ============================================================================
 * 语法声明
 * ============================================================================ */
    .syntax unified
    .cpu cortex-m3
    .fpu softvfp
    .thumb

/* ============================================================================
 * 全局符号定义
 * ============================================================================ */
    .global  Reset_Handler
    .global  Default_Handler

/* ============================================================================
 * 外部符号引用
 * ============================================================================ */
    .extern  main                    /* 用户main函数 */
    .extern  SystemInit              /* 系统初始化函数(可选) */
    .extern  _estack                 /* 栈顶地址，由链接脚本提供 */
    .extern  _sidata                 /* 数据段在Flash中的起始地址 */
    .extern  _sdata                  /* 数据段在RAM中的起始地址 */
    .extern  _edata                  /* 数据段在RAM中的结束地址 */
    .extern  _sbss                   /* BSS段在RAM中的起始地址 */
    .extern  _ebss                   /* BSS段在RAM中的结束地址 */

/* ============================================================================
 * 中断向量表
 * ============================================================================ */
    .section .isr_vector, "a", %progbits
    .type    g_pfnVectors, %object
    .size    g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
    .word    _estack                 /* 0: 初始栈指针 */
    .word    Reset_Handler           /* 1: 复位处理程序 */
    .word    NMI_Handler             /* 2: 不可屏蔽中断 */
    .word    HardFault_Handler       /* 3: 硬件故障 */
    .word    MemManage_Handler       /* 4: 内存管理故障 */
    .word    BusFault_Handler        /* 5: 总线故障 */
    .word    UsageFault_Handler      /* 6: 使用故障 */
    .word    0                       /* 7: 保留 */
    .word    0                       /* 8: 保留 */
    .word    0                       /* 9: 保留 */
    .word    0                       /* 10: 保留 */
    .word    SVC_Handler             /* 11: SVCall */
    .word    DebugMon_Handler        /* 12: 调试监视器 */
    .word    0                       /* 13: 保留 */
    .word    PendSV_Handler          /* 14: PendSV */
    .word    SysTick_Handler         /* 15: SysTick */

    /* STM32F103C8T6外部中断向量 */
    .word    WWDG_IRQHandler         /* 16: 窗口看门狗 */
    .word    PVD_IRQHandler          /* 17: PVD通过EXTI检测 */
    .word    TAMPER_IRQHandler       /* 18: 篡改检测 */
    .word    RTC_IRQHandler          /* 19: RTC全局中断 */
    .word    FLASH_IRQHandler        /* 20: Flash全局中断 */
    .word    RCC_IRQHandler          /* 21: RCC全局中断 */
    .word    EXTI0_IRQHandler        /* 22: EXTI线0中断 */
    .word    EXTI1_IRQHandler        /* 23: EXTI线1中断 */
    .word    EXTI2_IRQHandler        /* 24: EXTI线2中断 */
    .word    EXTI3_IRQHandler        /* 25: EXTI线3中断 */
    .word    EXTI4_IRQHandler        /* 26: EXTI线4中断 */
    .word    DMA1_Channel1_IRQHandler /* 27: DMA1通道1全局中断 */
    .word    DMA1_Channel2_IRQHandler /* 28: DMA1通道2全局中断 */
    .word    DMA1_Channel3_IRQHandler /* 29: DMA1通道3全局中断 */
    .word    DMA1_Channel4_IRQHandler /* 30: DMA1通道4全局中断 */
    .word    DMA1_Channel5_IRQHandler /* 31: DMA1通道5全局中断 */
    .word    DMA1_Channel6_IRQHandler /* 32: DMA1通道6全局中断 */
    .word    DMA1_Channel7_IRQHandler /* 33: DMA1通道7全局中断 */
    .word    ADC1_2_IRQHandler       /* 34: ADC1和ADC2全局中断 */
    .word    USB_HP_CAN1_TX_IRQHandler /* 35: USB高优先级或CAN1发送 */
    .word    USB_LP_CAN1_RX0_IRQHandler /* 36: USB低优先级或CAN1接收0 */
    .word    CAN1_RX1_IRQHandler     /* 37: CAN1接收1 */
    .word    CAN1_SCE_IRQHandler     /* 38: CAN1 SCE */
    .word    EXTI9_5_IRQHandler      /* 39: EXTI线[9:5]中断 */
    .word    TIM1_BRK_IRQHandler     /* 40: TIM1刹车中断 */
    .word    TIM1_UP_IRQHandler      /* 41: TIM1更新中断 */
    .word    TIM1_TRG_COM_IRQHandler /* 42: TIM1触发和通信中断 */
    .word    TIM1_CC_IRQHandler      /* 43: TIM1捕获比较中断 */
    .word    TIM2_IRQHandler         /* 44: TIM2全局中断 */
    .word    TIM3_IRQHandler         /* 45: TIM3全局中断 */
    .word    TIM4_IRQHandler         /* 46: TIM4全局中断 */
    .word    I2C1_EV_IRQHandler      /* 47: I2C1事件中断 */
    .word    I2C1_ER_IRQHandler      /* 48: I2C1错误中断 */
    .word    I2C2_EV_IRQHandler      /* 49: I2C2事件中断 */
    .word    I2C2_ER_IRQHandler      /* 50: I2C2错误中断 */
    .word    SPI1_IRQHandler         /* 51: SPI1全局中断 */
    .word    SPI2_IRQHandler         /* 52: SPI2全局中断 */
    .word    USART1_IRQHandler       /* 53: USART1全局中断 */
    .word    USART2_IRQHandler       /* 54: USART2全局中断 */
    .word    USART3_IRQHandler       /* 55: USART3全局中断 */
    .word    EXTI15_10_IRQHandler    /* 56: EXTI线[15:10]中断 */
    .word    RTCAlarm_IRQHandler     /* 57: RTC闹钟通过EXTI检测 */
    .word    USBWakeUp_IRQHandler    /* 58: USB唤醒通过EXTI检测 */
    .word    0                       /* 59: 保留 */
    .word    0                       /* 60: 保留 */
    .word    0                       /* 61: 保留 */
    .word    0                       /* 62: 保留 */
    .word    0                       /* 63: 保留 */
    .word    0                       /* 64: 保留 */
    .word    0                       /* 65: 保留 */
    .word    0                       /* 66: 保留 */
    .word    0                       /* 67: 保留 */
    .word    0                       /* 68: 保留 */
    .word    0                       /* 69: 保留 */
    .word    0                       /* 70: 保留 */
    .word    0                       /* 71: 保留 */
    .word    0                       /* 72: 保留 */
    .word    0                       /* 73: 保留 */
    .word    0                       /* 74: 保留 */
    .word    0                       /* 75: 保留 */
    .word    0                       /* 76: 保留 */
    .word    0                       /* 77: 保留 */
    .word    0                       /* 78: 保留 */
    .word    0                       /* 79: 保留 */
    .word    0                       /* 80: 保留 */
    .word    0                       /* 81: 保留 */
    .word    0                       /* 82: 保留 */
    .word    0                       /* 83: 保留 */
    .word    0                       /* 84: 保留 */
    .word    0                       /* 85: 保留 */
    .word    0                       /* 86: 保留 */
    .word    0                       /* 87: 保留 */
    .word    0                       /* 88: 保留 */
    .word    0                       /* 89: 保留 */
    .word    0                       /* 90: 保留 */
    .word    0                       /* 91: 保留 */
    .word    0                       /* 92: 保留 */
    .word    0                       /* 93: 保留 */
    .word    0                       /* 94: 保留 */
    .word    0                       /* 95: 保留 */
    .word    0                       /* 96: 保留 */
    .word    0                       /* 97: 保留 */
    .word    0                       /* 98: 保留 */
    .word    0                       /* 99: 保留 */
    .word    0                       /* 100: 保留 */
    .word    0                       /* 101: 保留 */
    .word    0                       /* 102: 保留 */
    .word    0                       /* 103: 保留 */
    .word    0                       /* 104: 保留 */
    .word    0                       /* 105: 保留 */
    .word    0                       /* 106: 保留 */
    .word    0                       /* 107: 保留 */
    .word    0                       /* 108: 保留 */
    .word    0                       /* 109: 保留 */
    .word    0                       /* 110: 保留 */
    .word    0                       /* 111: 保留 */
    .word    0                       /* 112: 保留 */
    .word    0                       /* 113: 保留 */
    .word    0                       /* 114: 保留 */
    .word    0                       /* 115: 保留 */
    .word    0                       /* 116: 保留 */
    .word    0                       /* 117: 保留 */
    .word    0                       /* 118: 保留 */
    .word    0                       /* 119: 保留 */
    .word    0                       /* 120: 保留 */
    .word    0                       /* 121: 保留 */
    .word    0                       /* 122: 保留 */
    .word    0                       /* 123: 保留 */
    .word    0                       /* 124: 保留 */
    .word    0                       /* 125: 保留 */
    .word    0                       /* 126: 保留 */
    .word    0                       /* 127: 保留 */

/* ============================================================================
 * 复位处理程序
 * ============================================================================ */
    .section .text.Reset_Handler
    .weak    Reset_Handler
    .type    Reset_Handler, %function

Reset_Handler:
    /* 复制数据段从Flash到RAM */
    ldr     r0, =_sdata              /* 数据段RAM起始地址 */
    ldr     r1, =_edata              /* 数据段RAM结束地址 */
    ldr     r2, =_sidata             /* 数据段Flash起始地址 */
    
    /* 检查是否有数据需要复制 */
    cmp     r0, r1
    beq     LoopCopyDataEnd
    
CopyDataLoop:
    ldr     r3, [r2], #4             /* 从Flash读取数据 */
    str     r3, [r0], #4             /* 写入RAM */
    cmp     r0, r1
    bne     CopyDataLoop

LoopCopyDataEnd:

    /* 初始化BSS段为零 */
    ldr     r0, =_sbss               /* BSS段起始地址 */
    ldr     r1, =_ebss               /* BSS段结束地址 */
    mov     r2, #0                   /* 清零值 */
    
    /* 检查是否有BSS需要清零 */
    cmp     r0, r1
    beq     LoopFillZerobssEnd
    
FillZerobssLoop:
    str     r2, [r0], #4             /* 写入零 */
    cmp     r0, r1
    bne     FillZerobssLoop

LoopFillZerobssEnd:

    /* 调用SystemInit函数(如果存在) */
    /* 注意: SystemInit函数需要在用户代码中实现 */
    /* 用于初始化系统时钟等 */
    bl      SystemInit

    /* 调用main函数 */
    bl      main

    /* 如果main函数返回，则无限循环 */
LoopForever:
    b       LoopForever

    .size    Reset_Handler, .-Reset_Handler

/* ============================================================================
 * 默认中断处理程序
 * ============================================================================ */
    .section .text.Default_Handler, "ax", %progbits
    .weak    Default_Handler
    .type    Default_Handler, %function

Default_Handler:
    /* 无限循环 */
    b       Default_Handler
    .size    Default_Handler, .-Default_Handler

/* ============================================================================
 * Cortex-M3内核中断处理程序(弱定义)
 * ============================================================================ */
    .weak    NMI_Handler
    .thumb_set NMI_Handler, Default_Handler

    .weak    HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler

    .weak    MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler

    .weak    BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler

    .weak    UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler

    .weak    SVC_Handler
    .thumb_set SVC_Handler, Default_Handler

    .weak    DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler

    .weak    PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler

    .weak    SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler

/* ============================================================================
 * STM32F103C8T6外部中断处理程序(弱定义)
 * ============================================================================ */
    .weak    WWDG_IRQHandler
    .thumb_set WWDG_IRQHandler, Default_Handler

    .weak    PVD_IRQHandler
    .thumb_set PVD_IRQHandler, Default_Handler

    .weak    TAMPER_IRQHandler
    .thumb_set TAMPER_IRQHandler, Default_Handler

    .weak    RTC_IRQHandler
    .thumb_set RTC_IRQHandler, Default_Handler

    .weak    FLASH_IRQHandler
    .thumb_set FLASH_IRQHandler, Default_Handler

    .weak    RCC_IRQHandler
    .thumb_set RCC_IRQHandler, Default_Handler

    .weak    EXTI0_IRQHandler
    .thumb_set EXTI0_IRQHandler, Default_Handler

    .weak    EXTI1_IRQHandler
    .thumb_set EXTI1_IRQHandler, Default_Handler

    .weak    EXTI2_IRQHandler
    .thumb_set EXTI2_IRQHandler, Default_Handler

    .weak    EXTI3_IRQHandler
    .thumb_set EXTI3_IRQHandler, Default_Handler

    .weak    EXTI4_IRQHandler
    .thumb_set EXTI4_IRQHandler, Default_Handler

    .weak    DMA1_Channel1_IRQHandler
    .thumb_set DMA1_Channel1_IRQHandler, Default_Handler

    .weak    DMA1_Channel2_IRQHandler
    .thumb_set DMA1_Channel2_IRQHandler, Default_Handler

    .weak    DMA1_Channel3_IRQHandler
    .thumb_set DMA1_Channel3_IRQHandler, Default_Handler

    .weak    DMA1_Channel4_IRQHandler
    .thumb_set DMA1_Channel4_IRQHandler, Default_Handler

    .weak    DMA1_Channel5_IRQHandler
    .thumb_set DMA1_Channel5_IRQHandler, Default_Handler

    .weak    DMA1_Channel6_IRQHandler
    .thumb_set DMA1_Channel6_IRQHandler, Default_Handler

    .weak    DMA1_Channel7_IRQHandler
    .thumb_set DMA1_Channel7_IRQHandler, Default_Handler

    .weak    ADC1_2_IRQHandler
    .thumb_set ADC1_2_IRQHandler, Default_Handler

    .weak    USB_HP_CAN1_TX_IRQHandler
    .thumb_set USB_HP_CAN1_TX_IRQHandler, Default_Handler

    .weak    USB_LP_CAN1_RX0_IRQHandler
    .thumb_set USB_LP_CAN1_RX0_IRQHandler, Default_Handler

    .weak    CAN1_RX1_IRQHandler
    .thumb_set CAN1_RX1_IRQHandler, Default_Handler

    .weak    CAN1_SCE_IRQHandler
    .thumb_set CAN1_SCE_IRQHandler, Default_Handler

    .weak    EXTI9_5_IRQHandler
    .thumb_set EXTI9_5_IRQHandler, Default_Handler

    .weak    TIM1_BRK_IRQHandler
    .thumb_set TIM1_BRK_IRQHandler, Default_Handler

    .weak    TIM1_UP_IRQHandler
    .thumb_set TIM1_UP_IRQHandler, Default_Handler

    .weak    TIM1_TRG_COM_IRQHandler
    .thumb_set TIM1_TRG_COM_IRQHandler, Default_Handler

    .weak    TIM1_CC_IRQHandler
    .thumb_set TIM1_CC_IRQHandler, Default_Handler

    .weak    TIM2_IRQHandler
    .thumb_set TIM2_IRQHandler, Default_Handler

    .weak    TIM3_IRQHandler
    .thumb_set TIM3_IRQHandler, Default_Handler

    .weak    TIM4_IRQHandler
    .thumb_set TIM4_IRQHandler, Default_Handler

    .weak    I2C1_EV_IRQHandler
    .thumb_set I2C1_EV_IRQHandler, Default_Handler

    .weak    I2C1_ER_IRQHandler
    .thumb_set I2C1_ER_IRQHandler, Default_Handler

    .weak    I2C2_EV_IRQHandler
    .thumb_set I2C2_EV_IRQHandler, Default_Handler

    .weak    I2C2_ER_IRQHandler
    .thumb_set I2C2_ER_IRQHandler, Default_Handler

    .weak    SPI1_IRQHandler
    .thumb_set SPI1_IRQHandler, Default_Handler

    .weak    SPI2_IRQHandler
    .thumb_set SPI2_IRQHandler, Default_Handler

    .weak    USART1_IRQHandler
    .thumb_set USART1_IRQHandler, Default_Handler

    .weak    USART2_IRQHandler
    .thumb_set USART2_IRQHandler, Default_Handler

    .weak    USART3_IRQHandler
    .thumb_set USART3_IRQHandler, Default_Handler

    .weak    EXTI15_10_IRQHandler
    .thumb_set EXTI15_10_IRQHandler, Default_Handler

    .weak    RTCAlarm_IRQHandler
    .thumb_set RTCAlarm_IRQHandler, Default_Handler

    .weak    USBWakeUp_IRQHandler
    .thumb_set USBWakeUp_IRQHandler, Default_Handler

/* ============================================================================
 * 文件结束
 * ============================================================================ */
    .end
