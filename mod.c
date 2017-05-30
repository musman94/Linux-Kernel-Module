#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/path.h>
#include <linux/dcache.h>
#include <linux/namei.h>

typedef struct PList
{
    struct task_struct* task;
    struct PList* next;
}PList;

int id;
module_param(id,int,0);

void processInfo(struct task_struct* task, int n)
{
    int count = 0;
    struct PList* head = kmalloc(sizeof(PList),GFP_KERNEL);
    if(head == NULL)
    {
    	printk("Error\n");
    	return 1;
    }
    head -> task = NULL;
    head -> next = NULL;
    struct PList* cur = head;
    struct list_head* pos;
    list_for_each(pos, &task->children)
    {
        if (head -> task == NULL)
            head -> task = list_entry(pos, struct task_struct, sibling);
        else
        {
            cur -> next = kmalloc(sizeof(PList),GFP_KERNEL);
            if(cur -> next == NULL)
            {
            	printk("Error\n");
    			return 1;
            }
            cur -> next -> task = list_entry(pos, struct task_struct, sibling);
            cur -> next -> next = NULL;
            cur = cur -> next;
        }
        count++;
    }
    printk("Process: %s[%d], Parent: %s[%d]\n", task -> comm, task -> pid , task -> parent -> comm, task -> parent -> pid);
    if(count > 0)
    {
        struct PList* pr;
        n = n - 1;
        int i = 1;
        if(n > 0)
            for(pr = head; pr != NULL;)
            {
                int m = 10000;
                for(; m > n; m--)
                {
                    printk("\t");
                }
                printk("--->Child: %d, ", i);
                processInfo(pr -> task, n);
                i = i+1;
                struct PList* temp = pr;
                pr = pr -> next;
                kfree(temp);
                temp = NULL;
            }
    }	  
  return 0;
}

void memAndFileInfo(void)
{
	bool found = false;
	struct task_struct *task;
	struct task_struct *t = current;
	struct mm_struct *m;
	struct vm_area_struct *v;
	struct files_struct *open_files;
	struct fdtable *files_table; 
	struct kstat *ks;
	struct path files_path;
	unsigned long t_size = 0;
	rcu_read_lock();
	for_each_process(task)
	{
		task_lock(task);
		int iid = task -> pid;
		if(iid == id)
		{	
			t = task;
			found = true;
		}
		task_unlock(task);
				
	}
	rcu_read_unlock();
	if(found)
	{
		printk("//////VIRTUAL MEMORY INFORMATION//////\n\n");
		v = t -> mm -> mmap;
		if(v != NULL)
		{
			printk("Process: %s[%d]\n",t -> comm, t -> pid);
		while(v -> vm_next != NULL)
		{
			unsigned long size = v -> vm_end - v -> vm_start;
			t_size = t_size + size;
			printk("Start: 0x%lx, End: 0x%lx, Block Size: 0x%lx\n", v -> vm_start, v -> vm_end, size);
			v = v -> vm_next;
		}
		printk("Total size of virtual space is: 0x%lx\n",t_size);
		printk("\n");
		printk("//////OPEN FILES INFORMATION//////\n\n");
		printk("Process: %s[%d]\n",t -> comm, t -> pid);
		int i = 0;
		open_files = t -> files;
		files_table = files_fdtable(open_files);
		char *path;
		char *buf = (char*)kmalloc(10000*sizeof(char),GFP_KERNEL);
		if(buf == NULL)
		{
			printk("ERROR\n");
			return 1;
		}
		while(files_table -> fd[i] != NULL)
		{
			files_path = files_table -> fd[i] -> f_path;
			char* name = files_table-> fd[i] -> f_path.dentry -> d_iname;
			long long size = i_size_read(files_table-> fd[i] -> f_path.dentry -> d_inode);
			path = d_path(&files_path,buf,10000*sizeof(char));
			printk("Name: %s, FD: %d, Size: 0x%llx bytes, Path: %s\n", name, i, size , path);
			i++;
		}
		}
	}
	else
	{
		printk("ID not found\n");
	}

}



int i_mod(void)
{

	struct task_struct *task;
	task = &init_task;
	int n = 10000;
	printk("\n\n//////PROCESS TREE//////\n\n");
	processInfo(task,n);
	printk("\n\n"); 
	memAndFileInfo();
	return 0;
	
}

	
	
void e_mod(void)
{
	printk("Done\n");
}

module_init(i_mod);
module_exit(e_mod);

