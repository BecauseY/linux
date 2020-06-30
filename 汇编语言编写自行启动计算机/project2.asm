assume cs:code

code segment
start: 
       call copy_introduce
       call copy_boot_disk
       mov ax,4c00h
       int 21h
copy_introduce:
       mov bx,cs
       mov es,bx
       mov bx,offset introduce

       mov al,1
       mov ch,0
       mov cl,1
       mov dl,0
       mov dh,0
       mov ah,3
       int 13h
       ret
copy_boot_disk:
       mov bx,cs
       mov es,bx
       mov bx,offset Boot

       mov al,2
       mov ch,0
       mov cl,2
       mov dl,0
       mov dh,0
       mov ah,3
       int 13h
       ret
;======================================引导程序开始===========================
introduce:
	call save_old_int9
	call copy_Boot_fromdisk

	mov bx,0
	push bx
	mov bx,7e00h
	push bx
	retf
copy_Boot_fromdisk:
        mov bx,0
	mov es,bx
	mov bx,7e00h

	mov al,2
	mov ch,0
	mov cl,2
	mov dl,0
	mov dh,0
	mov ah,2
	int 13h

	ret
save_old_int9:
       mov bx,0
       mov es,bx

       push es:[9*4]
       pop es:[200h]
       push es:[9*4+2]
       pop es:[202h]
       ret

       db 512 dup (0)
introduce_end:nop
;=================================引导程序结束=====================================

;=================================主代码开始======================================
Boot:
       jmp Boot_start
;xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx定义数据xxxxxxxxxxxxxxxxxxxxxxxxxxxx
option0   db 'welcome to 2017301510052-zbw os',0
option1   db '      1) reset pc',0
option2   db '      2) start system',0
option3   db '      3) clock',0
option4   db '      4) set clock',0

address_option   dw offset option0 - offset Boot + 7e00h
                 dw offset option1 - offset Boot + 7e00h
                 dw offset option2 - offset Boot + 7e00h
                 dw offset option3 - offset Boot + 7e00h
                 dw offset option4 - offset Boot + 7e00h
timeinfo   db'Press (F1) to change color     Press (ESC) to return',0
settimeinfo  db'year/month/day/h/m/s',0
timestyle   db '00/00/00 00:00:00',0
timeadress  db 9,8,7,4,2,0
string_stack   db 12 dup ('0'),0
;xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     

Boot_start:
        mov bx,0b800h
        mov es,bx            ;es指向显存
        mov bx,0
        mov ds,bx

	call clear_screen
	call show_option

	jmp short choose_option
        
	mov ax,4c00h
        int 21h
;-----------------------
choose_option:
        call clear_buff
        
	mov ah,0
	int 16h

	cmp al,'1'
	je choose1
	cmp al,'2'
	je choose2
	cmp al,'3'
	je choose3
	cmp al,'4'
	je choose4

        jmp choose_option

choose1:
        mov di,160*3
        mov byte ptr es:[di],'1'

	mov bx,0ffffh
	push bx
	mov bx,0
	push bx
	retf

choose2:
        mov di,160*3
        mov byte ptr es:[di],'2'

	mov bx,0
	mov es,bx
	mov bx,7c00h

	mov al,1
	mov ch,0
	mov cl,1
	mov dl,80h     ;80h代表C盘
	mov dh,0
	mov ah,2
	int 13h

        mov bx,0
	push bx
	mov bx,7c00h
	push bx
	retf

choose3:
        mov di,160*3
        mov byte ptr es:[di],'3'
	call show_clock
        jmp Boot_start

choose4:
        mov di,160*3
        mov byte ptr es:[di],'4'
	call set_clock
        jmp Boot_start
;===================选择4的代码===========================
set_clock:
        call clear_screen
        call clear_string_stack
        call show_string_stack
	call get_string
	call set_time
	ret
;--------------------
set_time:
        mov bx,offset timeadress - offset Boot + 7e00h
	mov si,offset string_stack - offset Boot +7e00h
	mov cx,6
set_time_loop:
        mov dx,ds:[si]
	sub dh,30h
	sub dl,30h
	shl dl,1
	shl dl,1
	shl dl,1
	shl dl,1
	and dh,00001111b
	or dl,dh
	mov al,ds:[bx]
	out 70h,al
	mov al,dl
	out 71h,al
        add si,2
	inc bx
	loop set_time_loop
        ret
;-------------------------
get_string:
        mov si,offset string_stack - offset Boot + 7e00h
	mov bx,0
getstring:
        call clear_buff
	mov ah,0
	int 16h
	cmp al,'0'
	jb notnumber
	cmp al,'9'
	ja notnumber
	call char_push
	call show_string_stack

	jmp getstring
getstring_end:
        ret
notnumber:
        cmp ah,0eh
	je isbackspace ;判断是否是删除键
	cmp ah,1ch
	je getstring_end ;判断是否是回车键
	jmp getstring
isbackspace:
        call char_pop
	call show_string_stack
        jmp getstring
;--------------------------
char_pop:
        cmp bx,0
	je charpopret
	dec bx
	mov byte ptr ds:[si+bx],'0'
charpopret:
        ret
;-------------------------
char_push:
        cmp bx,11
	ja charpushret
	mov ds:[si+bx],al
	inc bx
charpushret:
        ret
;-------------------
show_string_stack:
        push si
	push di
        mov si,offset settimeinfo - offset Boot + 7e00h
        mov di,160*10+25*2
	call showstr
	mov si,offset string_stack - offset Boot + 7e00h
        mov di,160*12+30*2
	call showstr
	pop di
	pop si
	ret
;--------------------
clear_string_stack:
        push bx
	push cx
	push es
	push si
	push di

        mov si,offset string_stack - offset Boot + 7e00h
	mov dx,3030h

	mov cx,6
clear_string_stack_loop:
        mov ds:[si],dx
	add si,2
	loop clear_string_stack_loop

	pop di
	pop si
	pop es
	pop cx
	pop bx
	ret
;=====================选择4代码结束==================

;=====================选择3的代码=====================
show_clock:
        call clear_screen
        call show_info
	call set_new_int9
	mov bx,offset timeadress - offset Boot + 7e00h
show_clock_time:       
	mov si,bx
	mov di,160*12+30*2
	mov cx,6
show_clock_time_loop:
        mov al,ds:[si]
	out 70h,al
	in al,71h

	mov ah,al
	shr ah,1
	shr ah,1
	shr ah,1
	shr ah,1
	and al,00001111b
	add ah,30h
	add al,30h
	mov es:[di],ah
	mov es:[di+2],al
	add di,6
	inc si
	loop show_clock_time_loop

        jmp show_clock_time

;----------------------
show_info:
       mov si,offset timeinfo - offset Boot + 7e00h
       mov di,160*10+15*2
       call showstr
       mov si,offset timestyle - offset Boot + 7e00h
       mov di,160*12+30*2
       call showstr
       ret
;--------------------
set_old_int9:
       push bx
       push es

       mov bx,0
       mov es,bx
       cli
       push es:[200h]
       pop es:[9*4]
       push es:[202h]
       pop es:[9*4+2]
       sti

       pop es
       pop bx
       ret
;---------------------
set_new_int9:
       push bx
       push es

       mov bx,0
       mov es,bx

       cli
       mov word ptr es:[9*4],offset newint9 - offset Boot + 7e00h
       mov word ptr es:[9*4+2],0
       sti
       
       pop es
       pop bx
       ret
;------------------------
newint9:
       push ax
       call clear_buff

       in al,60h
       pushf
       call dword ptr cs:[200h]
       
       cmp al,01h ;是不是esc
       je inesc
       cmp al,3bh ;是不是f1
       jne int9ret
       call change_time_color
       
int9ret:
       pop ax
       iret
inesc:
       pop ax
       add sp,4
       popf
       call set_old_int9
   ;----------------------
change_time_color:
       push bx
       push cx
       push es

       mov bx,0b800h
       mov es,bx
       mov cx,17
       mov bx,160*12+30*2+1
change_time_colors:
       inc byte ptr es:[bx]
       add bx,2
       loop change_time_colors

       pop es
       pop cx
       pop bx     
;================================选择3代码结束==========

clear_buff:
        ;下面两句调用BIOS的16号中断的01号功能，读取键盘状态。
        ;(1) 若无按键，零标志位版ZF←1
        ;(2) 若有按键，零标志位ZF←0，AH←键扫描码，AL←按权键字符ASCII码
        mov ah,1
	int 16h

	jz clearbuff_end ;JZ指令是在ZF=1时跳转，ZF=0时不跳转

        ;下面两句调用BIOS的16号中断的0号功能,从键盘缓冲区读取一个输入，并将其从缓冲区删除
	mov ah,0
	int 16h

	jmp clear_buff 
clearbuff_end:
	ret
;---------------------
show_option:
        mov bx,offset address_option - offset Boot + 7e00h
        mov cx,5
	mov di,160*9 + 25*2
show_option_loop:
        mov si,ds:[bx]
	call showstr
	add di,160
	add bx,2
	loop show_option_loop
        
	ret
;------------------------------
showstr:
        push cx  
	push di
showstr_loop:
	mov cl,ds:[si]
        cmp cl,0
	je showstr_end
	mov es:[di],cl
	add di,2
	inc si
	jmp short showstr_loop
showstr_end:
        pop di
        pop cx
	ret

;----------------
;显示缓冲区80*25=2000
;一个字符占用两个字节，分别放ASCII码和属性
clear_screen:
        push bx
        push dx
        push cx

        mov bx,0
        mov dx,0700h   ;清屏中对字符属性设置应该为07h，而不是0
        mov cx,2000 
clear_screen_loop:
       mov es:[bx],dx
       add bx,2
       loop clear_screen_loop

       pop cx
       pop dx
       pop bx
       ret



;-----------------------
       db 512 dup (0)
Boot_end:nop
;=================================主代码结束======================================
 code ends
  end start