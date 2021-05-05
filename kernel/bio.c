// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS    17

struct {
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf hash_bucket[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;

  for(int i = 0; i < NBUCKETS; i++) {
    initlock(&bcache.lock[i], "bcache");
    b = &bcache.hash_bucket[i];     // Point to itself for every bucket
    b->prev = b;
    b->next = b;
  }

  // Create linked list of buffers
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.hash_bucket[0].next;
    b->prev = &bcache.hash_bucket[0];
    initsleeplock(&b->lock, "buffer");
    bcache.hash_bucket[0].next->prev = b;
    bcache.hash_bucket[0].next = b;
  }
}

static int 
hash(int blockno) {
    return blockno % NBUCKETS;
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int id = hash(blockno);
  acquire(&bcache.lock[id]);
  
  // Is the block already cached?
  for(b = bcache.hash_bucket[id].next; b != &bcache.hash_bucket[id]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[id]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(int id_next = hash(id+1); id_next != id; id_next = hash(id_next+1)) {
      acquire( &bcache.lock[id_next] );
      for(b = bcache.hash_bucket[id_next].prev; b != &bcache.hash_bucket[id_next]; b = b->prev){
          if(b->refcnt == 0) {
              b->dev = dev;
              b->blockno = blockno;
              b->valid = 0;
              b->refcnt = 1;

              // 从原来的链表中断开
              b->next->prev = b->prev;
              b->prev->next = b->next;
              release( &bcache.lock[id_next] );
              
              // 把该节点插到blockno对应的bucket中
              b->next = bcache.hash_bucket[id].next;
              b->prev = &bcache.hash_bucket[id];
              bcache.hash_bucket[id].next->prev = b;
              bcache.hash_bucket[id].next = b;
              
              release( &bcache.lock[id] );
              acquiresleep(&b->lock);
              return b;
          }
      }
      release( &bcache.lock[id_next] );
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int id = hash( b->blockno );
  acquire(&bcache.lock[id]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hash_bucket[id].next;
    b->prev = &bcache.hash_bucket[id];
    bcache.hash_bucket[id].next->prev = b;
    bcache.hash_bucket[id].next = b;
  }
  
  release(&bcache.lock[id]);
}

void
bpin(struct buf *b) {
  int id = hash( b->blockno );
  acquire(&bcache.lock[id]);
  b->refcnt++;
  release(&bcache.lock[id]);
}

void
bunpin(struct buf *b) {
  int id = hash( b->blockno );
  acquire(&bcache.lock[id]);
  b->refcnt--;
  release(&bcache.lock[id]);
}


