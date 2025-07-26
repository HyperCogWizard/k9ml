/*
 * Cognitive Process Scheduler with Attention-Based Allocation
 * 
 * This module implements an attention-based cognitive process scheduler that
 * works alongside the traditional priority-based scheduler. It uses a tensor
 * of shape [n_procs, n_features, t_time] to track cognitive patterns and
 * allocate attention to processes based on their cognitive importance.
 */

#include	<u.h>
#include	"../port/lib.h"
#include	"mem.h"
#include	"dat.h"
#include	"fns.h"
#include	"../port/error.h"

/* Global attention allocator */
static AttentionAlloc attnalloc;
static int coginited = 0;

/* Cognitive feature types */
enum {
	CogFeatureLoad = 0,		/* CPU load pattern */
	CogFeatureMemory,		/* Memory access pattern */
	CogFeatureIO,			/* I/O activity pattern */
	CogFeatureInteractive,		/* Interactive response pattern */
	CogFeatureRealtime,		/* Real-time requirement pattern */
	CogFeatureNetwork,		/* Network activity pattern */
	CogFeaturePriority,		/* Traditional priority influence */
	CogFeatureEmergent,		/* Emergent behavioral pattern */
};

/* Attention calculation weights */
static float attention_weights[CogMaxFeatures] = {
	0.2,	/* Load */
	0.15,	/* Memory */
	0.1,	/* IO */
	0.25,	/* Interactive */
	0.15,	/* Realtime */
	0.05,	/* Network */
	0.05,	/* Priority */
	0.05,	/* Emergent */
};

static void cogqueueproc(CogSchedq*, Proc*);
static Proc* cogdequeueproc(CogSchedq*, Proc*);
static void cogcalcattention(Proc*);
static void cogupdatetensor(void);
static float coggetfeature(Proc*, int);
static void cogsetfeatureinternal(Proc*, int, float);

/*
 * Initialize the cognitive scheduler
 */
void
coginit(void)
{
	int i, j, k;
	CogSchedq *q;
	
	if(coginited)
		return;
		
	lock(&attnalloc);
	
	/* Initialize attention queues with different thresholds */
	for(i = 0; i < CogAttentionLevels; i++){
		q = &attnalloc.queues[i];
		q->head = nil;
		q->tail = nil;
		q->n = 0;
		q->attention_threshold = (float)(CogAttentionLevels - i) / CogAttentionLevels;
	}
	
	/* Initialize tensor to zero */
	lock(&attnalloc.tensor);
	for(i = 0; i < CogMaxProcs; i++){
		for(j = 0; j < CogMaxFeatures; j++){
			for(k = 0; k < CogTimeWindow; k++){
				attnalloc.tensor.data[i][j][k] = 0.0;
			}
		}
	}
	
	/* Initialize timestamps */
	for(i = 0; i < CogTimeWindow; i++){
		attnalloc.tensor.timestamp[i] = 0;
	}
	
	attnalloc.tensor.current_time = 0;
	attnalloc.tensor.active_procs = 0;
	unlock(&attnalloc.tensor);
	
	attnalloc.total_attention = 1000; /* Base attention units */
	attnalloc.last_update = 0;
	attnalloc.emergency_mode = 0;
	
	unlock(&attnalloc);
	coginited = 1;
}

/*
 * Add a process to cognitive scheduling
 */
void
cogready(Proc *p)
{
	int i;
	CogSchedq *q;
	
	if(!coginited)
		coginit();
		
	if(!p->is_cognitive)
		return;
		
	/* Update attention level */
	cogcalcattention(p);
	
	/* Find appropriate attention queue */
	for(i = 0; i < CogAttentionLevels; i++){
		q = &attnalloc.queues[i];
		if(p->attention_level >= q->attention_threshold){
			cogqueueproc(q, p);
			break;
		}
	}
	
	/* If no queue matched, use lowest priority queue */
	if(i == CogAttentionLevels){
		cogqueueproc(&attnalloc.queues[CogAttentionLevels-1], p);
	}
}

/*
 * Select next process from cognitive scheduler
 */
Proc*
cogrunproc(void)
{
	int i;
	CogSchedq *q;
	Proc *p;
	
	if(!coginited)
		return nil;
		
	/* Try each attention level from highest to lowest */
	for(i = 0; i < CogAttentionLevels; i++){
		q = &attnalloc.queues[i];
		if(q->head){
			p = cogdequeueproc(q, q->head);
			if(p){
				p->last_attention_update = MACHP(0)->ticks;
				return p;
			}
		}
	}
	
	return nil;
}

/*
 * Update cognitive features for a process
 */
void
cogupdate(Proc *p)
{
	ulong now, delta;
	float load_factor, mem_factor, interactive_factor;
	
	if(!p->is_cognitive)
		return;
		
	now = MACHP(0)->ticks;
	delta = now - p->last_attention_update;
	
	if(delta == 0)
		return;
		
	/* Calculate dynamic features */
	load_factor = (float)p->cpu / 1000.0;
	mem_factor = 0.5; /* Simplified - would analyze actual memory patterns */
	interactive_factor = (delta < HZ/10) ? 1.0 : 0.1; /* Recent activity = interactive */
	
	/* Update cognitive features */
	cogsetfeatureinternal(p, CogFeatureLoad, load_factor);
	cogsetfeatureinternal(p, CogFeatureMemory, mem_factor);
	cogsetfeatureinternal(p, CogFeatureInteractive, interactive_factor);
	cogsetfeatureinternal(p, CogFeaturePriority, (float)p->priority / Npriq);
	
	/* Update tensor with new data */
	cogupdatetensor();
	
	p->last_attention_update = now;
}

/*
 * Get current attention level for a process
 */
float
cogattention(Proc *p)
{
	if(!p->is_cognitive)
		return 0.0;
		
	return p->attention_level;
}

/*
 * Set a cognitive feature for a process
 */
void
cogsetfeature(Proc *p, int feature, float value)
{
	if(!p->is_cognitive || feature < 0 || feature >= CogMaxFeatures)
		return;
		
	cogsetfeatureinternal(p, feature, value);
}

/*
 * Check if process uses cognitive scheduling
 */
int
iscognitive(Proc *p)
{
	return p->is_cognitive;
}

/*
 * Set emergency mode (disables cognitive scheduling)
 */
void
cogemergency(int enable)
{
	lock(&attnalloc);
	attnalloc.emergency_mode = enable;
	unlock(&attnalloc);
}

/*
 * Debug dump of cognitive scheduler state
 */
void
cogdump(void)
{
	int i;
	CogSchedq *q;
	Proc *p;
	
	if(!coginited){
		print("Cognitive scheduler not initialized\n");
		return;
	}
	
	print("Cognitive Scheduler State:\n");
	print("Total attention: %lud, Emergency mode: %d\n", 
		attnalloc.total_attention, attnalloc.emergency_mode);
	print("Active processes in tensor: %d\n", attnalloc.tensor.active_procs);
	
	for(i = 0; i < CogAttentionLevels; i++){
		q = &attnalloc.queues[i];
		if(q->n > 0){
			print("Attention queue %d (threshold %.2f): %d processes\n", 
				i, q->attention_threshold, q->n);
			for(p = q->head; p; p = p->cog_next){
				print("  Process %lud: attention=%.2f features=[%.2f,%.2f,%.2f,%.2f]\n",
					p->pid, p->attention_level,
					p->cognitive_features[0], p->cognitive_features[1],
					p->cognitive_features[2], p->cognitive_features[3]);
			}
		}
	}
}

/*
 * Internal functions
 */

static void
cogqueueproc(CogSchedq *q, Proc *p)
{
	lock(q);
	p->cog_next = nil;
	if(q->tail)
		q->tail->cog_next = p;
	else
		q->head = p;
	q->tail = p;
	q->n++;
	unlock(q);
}

static Proc*
cogdequeueproc(CogSchedq *q, Proc *tp)
{
	Proc *l, *p;
	
	if(!canlock(q))
		return nil;
		
	/* Find the target process */
	l = nil;
	for(p = q->head; p; p = p->cog_next){
		if(p == tp)
			break;
		l = p;
	}
	
	if(p == nil){
		unlock(q);
		return nil;
	}
	
	/* Remove from queue */
	if(p->cog_next == nil)
		q->tail = l;
	if(l)
		l->cog_next = p->cog_next;
	else
		q->head = p->cog_next;
		
	q->n--;
	unlock(q);
	return p;
}

static void
cogcalcattention(Proc *p)
{
	int i;
	float attention = 0.0;
	
	/* Calculate weighted sum of cognitive features */
	for(i = 0; i < CogMaxFeatures; i++){
		attention += p->cognitive_features[i] * attention_weights[i];
	}
	
	/* Apply emergent behavior modifier */
	if(p->cognitive_features[CogFeatureEmergent] > 0.5){
		attention *= 1.2; /* Boost emergent processes */
	}
	
	/* Clamp to valid range */
	if(attention > 1.0)
		attention = 1.0;
	if(attention < 0.0)
		attention = 0.0;
		
	p->attention_level = attention;
}

static void
cogupdatetensor(void)
{
	/* Simplified tensor update - in a full implementation this would
	 * update the temporal patterns in the attention tensor */
	lock(&attnalloc.tensor);
	attnalloc.tensor.current_time = (attnalloc.tensor.current_time + 1) % CogTimeWindow;
	attnalloc.tensor.timestamp[attnalloc.tensor.current_time] = MACHP(0)->ticks;
	unlock(&attnalloc.tensor);
}

static float
coggetfeature(Proc *p, int feature)
{
	if(feature < 0 || feature >= CogMaxFeatures)
		return 0.0;
	return p->cognitive_features[feature];
}

static void
cogsetfeatureinternal(Proc *p, int feature, float value)
{
	if(feature < 0 || feature >= CogMaxFeatures)
		return;
		
	/* Clamp value to valid range */
	if(value > 1.0)
		value = 1.0;
	if(value < 0.0)
		value = 0.0;
		
	p->cognitive_features[feature] = value;
}