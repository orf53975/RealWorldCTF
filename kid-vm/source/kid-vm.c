#include <stdio.h>
#include <stdlib.h>
#include <linux/kvm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#define err_exit(x) do{perror((x));return 1;}while(0)
uint64_t memory_list[0x10];
uint32_t memory_size[0x10];
uint32_t num=0;

void hypercall_malloc(uint16_t args1){
	int i;
	uint64_t *ptr;
	if(num>=0x10){
		puts("Too many memory!");
		return;
	}
	if(args1>0x1000)
		err_exit("Too big!");
	if(args1<0x80)
		err_exit("Too small!");
	for(i=0;i<0x10;i++)
		if(!memory_list[i])
			break;
	ptr=malloc(args1);
	if(!ptr)
		err_exit("Malloc fail!");
	memory_list[i]=ptr;
	memory_size[i]=args1;
	num++;
	return;
}
void hypercall_free(uint16_t args1,uint16_t args2){
	if(args2>0x10)
		err_exit("Index out of bound!");
	switch(args1){
		case 1:
			free(memory_list[args2]);
			break;
		case 2:
			free(memory_list[args2]);
			memory_list[args2]=0;
      num--;
			break;
		case 3:
			free(memory_list[args2]);
			memory_list[args2]=0;
			memory_size[args2]=0;
      num--;
			break;
    default:
      break;
	}
	return;
}
void hypercall_data_transfer(uint16_t args1,uint16_t args2,uint16_t args3,uint64_t args4){
	if(args2>0x10)
		err_exit("Index out of bound!");
	if(!memory_list[args2])
		err_exit("No memory in this idx!");
	if(args3>memory_size[args2])
		err_exit("Memory overflow!");
	switch(args1){
		case 1:
			memcpy(memory_list[args2],args4+0x4000,args3);
			break;
		case 2:
			memcpy(args4+0x4000,memory_list[args2],args3);
			break;
    default:
      break;
	}
	return;
}
    const uint8_t code[] = {
  0xBC, 0x00, 0x30, 0xE8, 0x01, 0x00, 0xF4, 0xB8, 0x17, 0x02, 
  0xBB, 0x30, 0x00, 0xE8, 0xE3, 0x01, 0xB8, 0x47, 0x02, 0xBB, 
  0x94, 0x00, 0xE8, 0xDA, 0x01, 0xB8, 0x3E, 0x03, 0xBB, 0x01, 
  0x00, 0xE8, 0xE3, 0x01, 0xBE, 0x3E, 0x03, 0x8A, 0x04, 0x3C, 
  0x31, 0x74, 0x23, 0x3C, 0x32, 0x74, 0x24, 0x3C, 0x33, 0x74, 
  0x25, 0x3C, 0x34, 0x74, 0x26, 0x3C, 0x35, 0x74, 0x27, 0x3C, 
  0x36, 0x74, 0x28, 0x3C, 0x37, 0x74, 0x29, 0xB8, 0xDB, 0x02, 
  0xBB, 0x06, 0x00, 0xE8, 0xA7, 0x01, 0xEB, 0x1F, 0xE8, 0x1E, 
  0x00, 0xEB, 0x1A, 0xE8, 0x92, 0x00, 0xEB, 0x15, 0xE8, 0xCE, 
  0x00, 0xEB, 0x10, 0xE8, 0xCA, 0x00, 0xEB, 0x0B, 0xE8, 0xF2, 
  0x00, 0xEB, 0x06, 0xE8, 0x59, 0x01, 0xEB, 0x01, 0xC3, 0xEB, 
  0xA1, 0x50, 0x53, 0x51, 0x52, 0x56, 0x57, 0xB8, 0xE7, 0x02, 
  0xBB, 0x05, 0x00, 0xE8, 0x75, 0x01, 0xB8, 0x40, 0x03, 0xBB, 
  0x02, 0x00, 0xE8, 0x7E, 0x01, 0xA1, 0x40, 0x03, 0x3D, 0x00, 
  0x10, 0x77, 0x33, 0x8B, 0x0E, 0x44, 0x03, 0x81, 0xF9, 0x00, 
  0xB0, 0x77, 0x34, 0x8B, 0x36, 0x42, 0x03, 0x83, 0xFE, 0x10, 
  0x73, 0x36, 0x89, 0xCF, 0x81, 0xC1, 0x00, 0x50, 0x01, 0xF6, 
  0x89, 0x8C, 0x46, 0x03, 0x89, 0x84, 0x66, 0x03, 0x01, 0xC7, 
  0x89, 0x3E, 0x44, 0x03, 0xA0, 0x42, 0x03, 0xFE, 0xC0, 0xA2, 
  0x42, 0x03, 0xEB, 0x1F, 0xB8, 0xEC, 0x02, 0xBB, 0x08, 0x00, 
  0xE8, 0x28, 0x01, 0xEB, 0x14, 0xB8, 0xF4, 0x02, 0xBB, 0x32, 
  0x00, 0xE8, 0x1D, 0x01, 0xEB, 0x09, 0xB8, 0x26, 0x03, 0xBB, 
  0x10, 0x00, 0xE8, 0x12, 0x01, 0x5F, 0x5E, 0x5A, 0x59, 0x5B, 
  0x58, 0xC3, 0x50, 0x53, 0x51, 0x52, 0x56, 0x57, 0xB8, 0xE1, 
  0x02, 0xBB, 0x06, 0x00, 0xE8, 0xFC, 0x00, 0xB8, 0x43, 0x03, 
  0xBB, 0x01, 0x00, 0xE8, 0x05, 0x01, 0xB8, 0x36, 0x03, 0xBB, 
  0x08, 0x00, 0xE8, 0xEA, 0x00, 0xA0, 0x43, 0x03, 0x3A, 0x06, 
  0x42, 0x03, 0x73, 0x10, 0x0F, 0xB6, 0xF0, 0x01, 0xF6, 0x8B, 
  0x84, 0x46, 0x03, 0x8B, 0x9C, 0x66, 0x03, 0xE8, 0xE3, 0x00, 
  0x5F, 0x5E, 0x5A, 0x59, 0x5B, 0x58, 0xC3, 0xC3, 0x50, 0x53, 
  0x51, 0x52, 0x56, 0x57, 0xB8, 0xE7, 0x02, 0xBB, 0x05, 0x00, 
  0xE8, 0xBA, 0x00, 0xB8, 0x40, 0x03, 0xBB, 0x02, 0x00, 0xE8, 
  0xC3, 0x00, 0x68, 0x00, 0x01, 0x9D, 0x8B, 0x1E, 0x40, 0x03, 
  0xB8, 0x00, 0x01, 0x0F, 0x01, 0xC1, 0x5F, 0x5E, 0x5A, 0x59, 
  0x5B, 0x58, 0xC3, 0x50, 0x53, 0x51, 0x52, 0x56, 0x57, 0xB8, 
  0xE7, 0x02, 0xBB, 0x05, 0x00, 0xE8, 0x8D, 0x00, 0xB8, 0x40, 
  0x03, 0xBB, 0x02, 0x00, 0xE8, 0x96, 0x00, 0xA1, 0x40, 0x03, 
  0x3D, 0x00, 0x10, 0x77, 0x45, 0xB8, 0xE1, 0x02, 0xBB, 0x06, 
  0x00, 0xE8, 0x73, 0x00, 0xB8, 0x43, 0x03, 0xBB, 0x01, 0x00, 
  0xE8, 0x7C, 0x00, 0xB8, 0x36, 0x03, 0xBB, 0x08, 0x00, 0xE8, 
  0x61, 0x00, 0xB8, 0x00, 0x40, 0x8B, 0x1E, 0x40, 0x03, 0xE8, 
  0x69, 0x00, 0x68, 0x00, 0x01, 0x9D, 0xB8, 0x02, 0x01, 0xBB, 
  0x01, 0x00, 0x8A, 0x0E, 0x43, 0x03, 0x8B, 0x16, 0x40, 0x03, 
  0x0F, 0x01, 0xC1, 0xEB, 0x09, 0xB8, 0xEC, 0x02, 0xBB, 0x08, 
  0x00, 0xE8, 0x37, 0x00, 0x5F, 0x5E, 0x5A, 0x59, 0x5B, 0x58, 
  0xC3, 0x50, 0x53, 0x51, 0x52, 0x56, 0x57, 0xB8, 0xE1, 0x02, 
  0xBB, 0x06, 0x00, 0xE8, 0x21, 0x00, 0xB8, 0x43, 0x03, 0xBB, 
  0x01, 0x00, 0xE8, 0x2A, 0x00, 0x68, 0x00, 0x01, 0x9D, 0xB8, 
  0x01, 0x01, 0xBB, 0x03, 0x00, 0x8A, 0x0E, 0x43, 0x03, 0x0F, 
  0x01, 0xC1, 0x5F, 0x5E, 0x5A, 0x59, 0x5B, 0x58, 0xC3, 0x51, 
  0x52, 0x56, 0x89, 0xD9, 0x89, 0xC6, 0x8A, 0x04, 0xE6, 0x17, 
  0x46, 0xE2, 0xF9, 0x5E, 0x5A, 0x59, 0xC3, 0x51, 0x52, 0x56, 
  0x89, 0xD9, 0x89, 0xC6, 0xE4, 0x17, 0x88, 0x04, 0x46, 0xE2, 
  0xF9, 0x5E, 0x5A, 0x59, 0xC3, 0x57, 0x65, 0x6C, 0x63, 0x6F, 
  0x6D, 0x65, 0x20, 0x74, 0x6F, 0x20, 0x74, 0x68, 0x65, 0x20, 
  0x56, 0x69, 0x72, 0x74, 0x75, 0x61, 0x6C, 0x20, 0x57, 0x6F, 
  0x72, 0x6C, 0x64, 0x20, 0x4D, 0x65, 0x6D, 0x6F, 0x72, 0x79, 
  0x20, 0x4D, 0x61, 0x6E, 0x61, 0x67, 0x65, 0x6D, 0x65, 0x6E, 
  0x74, 0x21, 0x0A, 0x57, 0x68, 0x61, 0x74, 0x20, 0x64, 0x6F, 
  0x20, 0x79, 0x6F, 0x75, 0x20, 0x77, 0x61, 0x6E, 0x74, 0x20, 
  0x74, 0x6F, 0x20, 0x64, 0x6F, 0x3F, 0x0A, 0x31, 0x2E, 0x41, 
  0x6C, 0x6C, 0x6F, 0x63, 0x20, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 
  0x79, 0x0A, 0x32, 0x2E, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 
  0x20, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 0x79, 0x0A, 0x33, 0x2E, 
  0x46, 0x72, 0x65, 0x65, 0x20, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 
  0x79, 0x0A, 0x34, 0x2E, 0x41, 0x6C, 0x6C, 0x6F, 0x63, 0x20, 
  0x68, 0x6F, 0x73, 0x74, 0x20, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 
  0x79, 0x0A, 0x35, 0x2E, 0x55, 0x70, 0x64, 0x61, 0x74, 0x65, 
  0x20, 0x68, 0x6F, 0x73, 0x74, 0x20, 0x6D, 0x65, 0x6D, 0x6F, 
  0x72, 0x79, 0x0A, 0x36, 0x2E, 0x46, 0x72, 0x65, 0x65, 0x20, 
  0x68, 0x6F, 0x73, 0x74, 0x20, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 
  0x79, 0x0A, 0x37, 0x2E, 0x45, 0x78, 0x69, 0x74, 0x0A, 0x59, 
  0x6F, 0x75, 0x72, 0x20, 0x63, 0x68, 0x6F, 0x69, 0x63, 0x65, 
  0x3A, 0x57, 0x72, 0x6F, 0x6E, 0x67, 0x0A, 0x49, 0x6E, 0x64, 
  0x65, 0x78, 0x3A, 0x53, 0x69, 0x7A, 0x65, 0x3A, 0x54, 0x6F, 
  0x6F, 0x20, 0x62, 0x69, 0x67, 0x0A, 0x47, 0x75, 0x65, 0x73, 
  0x74, 0x20, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 0x79, 0x20, 0x69, 
  0x73, 0x20, 0x66, 0x75, 0x6C, 0x6C, 0x21, 0x20, 0x50, 0x6C, 
  0x65, 0x61, 0x73, 0x65, 0x20, 0x75, 0x73, 0x65, 0x20, 0x74, 
  0x68, 0x65, 0x20, 0x68, 0x6F, 0x73, 0x74, 0x20, 0x6D, 0x65, 
  0x6D, 0x6F, 0x72, 0x79, 0x21, 0x0A, 0x54, 0x6F, 0x6F, 0x20, 
  0x6D, 0x61, 0x6E, 0x79, 0x20, 0x6D, 0x65, 0x6D, 0x6F, 0x72, 
  0x79, 0x0A, 0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x3A, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00};
int main(int argc, char *argv[]) {
    int kvm, vmfd, vcpufd, ret,args1,args2,args3,func;
    uint64_t *ptr;
    char c;

    uint8_t *mem;
    struct kvm_sregs sregs;
    size_t mmap_size;
    struct kvm_run *run;
    setbuf(stdin,0);
    setbuf(stdout,0);
    num=0;
    kvm = open("/dev/kvm", O_RDWR|O_CLOEXEC);
    if (kvm==-1) {
        err_exit("Open /dev/kvm failed!\n");
    }

    ret = ioctl(kvm, KVM_GET_API_VERSION, NULL);
    if (ret==-1) {
        err_exit("KVM_GET_API_VERSION");
    } else if(ret!=12) {
        err_exit("KVM_GET_API_VERSION not 12");
    }

    ret = ioctl(kvm, KVM_CHECK_EXTENSION, KVM_CAP_USER_MEMORY);
    if (ret==-1) {
        err_exit("KVM_CHECK_EXTENSION");
    }
    if (!ret) {
        err_exit("Required extension KVM_CAP_USER_MEM not available");
    }


    vmfd = ioctl(kvm, KVM_CREATE_VM, (unsigned long)0);
    if (vmfd==-1) {
        err_exit("KVM_CREATE_VM");
    }


    mem = mmap(NULL, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if (!mem) {
        err_exit("allocation guest memory");
    }

    memcpy(mem, code, sizeof(code));

    struct kvm_userspace_memory_region region= {
        .slot = 0,
        .guest_phys_addr= 0,
        .memory_size = 0x10000,
        .userspace_addr = (uint64_t)mem,
    };

    ret = ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);
    if (ret==-1) {
        err_exit("KVM_SET_USER_MEMORY_REGION");
    }


    vcpufd= ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);
    if (vcpufd==-1) {
        err_exit("KVM_CREATE_VCPU");
    }

    ret = ioctl(kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
    if (ret==-1) {
        err_exit("KVM_GET_VCPU_MMAP_SIZE");
    }

    mmap_size = ret;
    if (mmap_size<sizeof(*run)) {
        err_exit("KVM_GET_VCPU_MMAP_SIZE unexpectedly small");
    }


    run = mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, vcpufd, 0);
    if (!run) {
        err_exit("mmap vcpu");
    }

    /* 设置cs寄存器 */
    ret = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    if (ret==-1) {
        err_exit("KVM_GET_SREGS");
    }
    sregs.cs.base = 0;
    sregs.cs.selector = 0;
    ret = ioctl(vcpufd, KVM_SET_SREGS, &sregs);
    if (ret==-1) {
        err_exit("KVM_SET_SREGS");
    }

    struct kvm_regs regs = {
        .rip = 0x0,
        .rax = 0,
        .rbx = 0,
        .rflags = 0x102,
    };
    ret = ioctl(vcpufd, KVM_SET_REGS, &regs);
    if (ret==-1) {
        err_exit("KVM_SET_REGS");
    }
    struct kvm_guest_debug debug = {
		.control	= KVM_GUESTDBG_ENABLE | KVM_GUESTDBG_SINGLESTEP,
	};

	if (ioctl(vcpufd, KVM_SET_GUEST_DEBUG, &debug) < 0){
		err_exit("KVM_SET_GUEST_DEBUG");
	}
    while(1) {
        ret = ioctl(vcpufd, KVM_RUN, NULL);
 /*       ret = ioctl(vcpufd, KVM_GET_REGS, &regs);
        if(ret==-1){
        	puts("Error get regs!");
        }
        printf("KVM_EXIT while rip=%llx,rax=%llx,rbx=%llx\n",regs.rip,regs.rax,regs.rbx);
*/
        if (ret==-1) {
            err_exit("KVM_RUN");
        }
        switch (run->exit_reason) {
        case KVM_EXIT_HLT:
            puts("KVM_EXIT_HLT");
            return 0;
        case KVM_EXIT_IO:

            if (run->io.direction==KVM_EXIT_IO_OUT && run->io.size==1 && run->io.port==0x17 && run->io.count==1) {
                putchar(*(((char *)run) + run->io.data_offset));
            } else if(run->io.direction==KVM_EXIT_IO_IN && run->io.size==1 && run->io.port==0x17 && run->io.count==1){
            	read(0,((char *)run) + run->io.data_offset,1);
            } 
        /*    else if(run->io.direction==KVM_EXIT_IO_OUT && run->io.size==1 && run->io.port==0x20 && run->io.count==1){
            	ret = ioctl(vcpufd, KVM_GET_REGS, &regs);
        		if(ret==-1){
        			puts("Error get regs!");
        		}
        		printf("KVM_EXIT while rip=%llx,rax=%llx,rbx=%llx\n",regs.rip,regs.rax,regs.rbx);
            }*/
            else{
                fprintf(stderr, "unhandled KVM_EXIT_IO\n");
                fprintf(stdout, "KVM_EXIT_IO direction:%d size:%d port:0x%x count:%d\n", run->io.direction,run->io.size, run->io.port, run->io.count);
                return 1;
            }
            break;
        case KVM_EXIT_DEBUG:
        	ret = ioctl(vcpufd, KVM_GET_REGS, &regs);
        	if(ret==-1){
        			puts("Error get regs!");
        	}
        	if(mem[regs.rip]==15&&mem[regs.rip+1]==1&&mem[regs.rip+2]==0xc1){
        		puts("Hypercall!");

            ret = ioctl(vcpufd, KVM_GET_REGS, &regs);
        		if(ret==-1){
        			puts("Error get regs!");
        		}
        		
        		func = regs.rax&0xffff;
        		args1 = regs.rbx&0xffff;
        		args2 = regs.rcx&0xffff;
        		args3 = regs.rdx&0xffff;
        		switch(func){
        			case 0x100:
        				hypercall_malloc(args1);
        				break;
        			case 0x101:
        				hypercall_free(args1,args2);
        				break;
        			case 0x102:
        				hypercall_data_transfer(args1,args2,args3,mem);
        				break;
        			default:
        				puts("Functino error!");
        				break;
        		}
        	}
        	break;
        case KVM_EXIT_FAIL_ENTRY:
            fprintf(stderr, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason=0x%llx\n",
                    (unsigned long long)run->fail_entry.hardware_entry_failure_reason);
            return 1;
        case KVM_EXIT_INTERNAL_ERROR:
            fprintf(stderr, "KVM_EXIT_INTERNAL_ERROR: suberror=0x%x\n", run->internal.suberror);
            return 1;
        default:
            fprintf(stderr, "exit_reason=0x%x\n", run->exit_reason);
            return 0;
        }
    }
}