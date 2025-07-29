#include <klee/klee.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct node
{
    int info;
    struct node *link;
};

struct node *start = NULL;

struct node *createnode()
{
    struct node *t;
    t = (struct node *)malloc(sizeof(struct node));
    return (t);
}

int insert(int pos, int d)
{
    struct node *new;
    new = createnode();
    new->info = d;
    if (pos == 1)
    {
        new->link = NULL;
        if (start == NULL)
        {
            start = new;
        }
        else
        {
            new->link = start;
            start = new;
        }
    }
    else
    {
        struct node *pre = start;
        for (int i = 2; i < pos; i++)
        {
             if (pre == NULL)
            {
                break;
            }
            pre = pre->link;
        }
        if(pre==NULL)
        {
            return 0;
        }
        new->link = pre->link;
        pre->link = new;
    }
    return 1;
}

int deletion(int pos)
{
    struct node *t;
    if (start == NULL)
    {
        return 0;
    }
    else
    {
        if (pos == 1)
        {
            struct node *p;
            p = start;
            start = start->link;
            free(p);
        }
        else
        {
            struct node *prev = start;
            for (int i = 2; i < pos; i++)
            {
                if (prev == NULL)
                {
                    return 0;
                }
                prev = prev->link;
            }
            if (prev->link == NULL) {
                return 0;
            }
            struct node *n = prev->link;
            prev->link = n->link;
            free(n);
        }
    }
    return 1;
}

int count_nodes()
{
    struct node *p = start;
    int count = 0;
    while (p != NULL)
    {
        count++;
        p = p->link;
    }
    return count;
}

int main()
{
    int pos, num;
    
    // Make inputs symbolic
    klee_make_symbolic(&pos, sizeof(pos), "position");
    klee_make_symbolic(&num, sizeof(num), "number");
    
    // Constrain inputs to reasonable values
    klee_assume(pos >= 1 && pos <= 10);
    klee_assume(num >= 0 && num <= 100);
    
    // Insert some initial elements
    insert(1, 10);
    insert(2, 20);
    insert(3, 30);
    
    // Verify initial state
    klee_assert(count_nodes() == 3);
    
    // Perform symbolic operations
    int insert_result = insert(pos, num);
    klee_assert(insert_result == 1);
    
    // Verify list grew by one
    klee_assert(count_nodes() == 4);
    
    // Try to delete at symbolic position
    int delete_result = deletion(pos);
    
    // Verify deletion was successful
    klee_assert(delete_result == 1);
    
    // Verify list shrunk by one
    klee_assert(count_nodes() == 3);
    
    return 0;
} 