/**
 * \file p3linux.c
 * <h3>Protected Point to Point Linux functions file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The Velocite Systems Protected Point to Point (P3) Linux functions
 * are specific to the Linux kernel.  These functions include:
 * <ul>
 *   <li>Initialization</li>
 *   <li>Memory mapping</li>
 *   <li>IO Control</li>
 *   <li>Packet interception</li>
 *   <li>Cleanup</li>
 * </ul>
 */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_KERNEL P3 System Kernel Module Messages
 */

#include "p3knet.h"

#ifndef _p3_SECONDARY
#include "p3kprimary.h"
#endif
#ifndef _p3_PRIMARY
#include "p3ksecondary.h"
#endif

static unsigned char *ramdisk;
static size_t ramdisk_size = RAMDISK_SZ;
static unsigned int count = 1;  /* number of dev_t needed */
static dev_t ramdisk_region;
static struct device *ramdisk_device = NULL;
static struct cdev *ramdisk_cdev;
static struct class *ramdisk_class;

/**
 * \par Function:
 * p3errmsg
 *
 * \par Description:
 * Handle error messages for the P3 system.  The types of messages are
 * defined in the p3kbase.h include file.  They are:
 * <ul>
 *  <li>Critical:  Caused by system errors</li>
 *  <li>Error:  Caused by application errors</li>
 *  <li>Warning:  Does not cause application shutdown, but indicate the
 *      need for adminstrative correction</li>
 *  <li>Notice:  Information that is always reported, such as startup</li>
 *  <li>Information:  Information such as usage statistics.  This may be
 *      ignored using the -I command line argument</li>
 * </ul>
 *
 * \par Inputs:
 * - type: The type of the message
 * - message: The text of the message
 *
 * \par Outputs:
 * - None
 */

void p3errmsg(int type, char *message)
{
	switch(type) {
		case p3MSG_CRIT:
			printk(KERN_CRIT "%s", message);
			break;

		case p3MSG_ERR:
			printk(KERN_ERR "%s", message);
			break;

		case p3MSG_WARN:
			printk(KERN_WARNING "%s", message);
			break;

		case p3MSG_NOTICE:
			printk(KERN_NOTICE "%s", message);
			break;

		case p3MSG_INFO:
			printk(KERN_INFO "%s", message);
			break;

		case p3MSG_DEBUG:
			printk(KERN_DEBUG "%s", message);
			break;

	}
}

/**
 * \par Function:
 * p3ramdisk_open
 *
 * \par Description:
 * The handler for opening the RAM disk being used for mmap functions.  This
 * is called by the kernel when the user space application opens the device.
 *
 * \par Inputs:
 * - inode: The inode of the RAM disk.
 * - file: The file structure of the RAM disk.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 */

static inline int p3ramdisk_open (struct inode *inode, struct file *file)
{
p3errmsg(p3MSG_DEBUG, "Open RAM disk\n");
	return 0;
}
/**
 * \par Function:
 * p3ramdisk_release
 *
 * \par Description:
 * The handler for closing the RAM disk being used for mmap functions.  This
 * is called by the kernel when the user space application closes the device.
 *
 * \par Inputs:
 * - inode: The inode of the RAM disk.
 * - file: The file structure of the RAM disk.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 */

static inline int ramdisk_release (struct inode *inode, struct file *file)
{
	return 0;
}

/**
 * \par Function:
 * p3ramdisk_read
 *
 * \par Description:
 * Handle a request to read from the RAM disk space.  This is called
 * by the kernel when the user space application reads from the device.
 *
 * \par Inputs:
 * - file: The file structure of the RAM disk.
 * - buf: The user space buffer location.
 * - bufsz: The length of the user space buffer.
 * - pos: The starting position in the RAM disk to begin reading.
 *
 * \par Outputs:
 * - int: Number of bytes read.
 */

static inline ssize_t
p3ramdisk_read (struct file *file, char __user *buf, size_t bufsz, loff_t *pos)
{
	int copysz, actsz, reqsz;

	actsz = ramdisk_size - *pos;
	reqsz = actsz > bufsz ? bufsz : actsz;
	if (reqsz == 0) {
		*pos = ramdisk_size;
		return 0;
	}
	copysz = reqsz - copy_to_user (buf, ramdisk + *pos, reqsz);
	*pos += copysz;
	return copysz;
}

/**
 * \par Function:
 * p3ramdisk_write
 *
 * \par Description:
 * Handle a request to write to the RAM disk space.  This is called
 * by the kernel when the user space application writes to the device.
 *
 * \par Inputs:
 * - file: The file structure of the RAM disk.
 * - buf: The user space buffer location.
 * - bufsz: The length of the user space buffer.
 * - pos: The starting position in the RAM disk to begin writing.
 *
 * \par Outputs:
 * - int: Number of bytes written.
 */

static inline ssize_t
p3ramdisk_write (struct file *file, const char __user *buf, size_t bufsz, loff_t *pos)
{
	int copysz, actsz, reqsz;

	actsz = ramdisk_size - *pos;
	reqsz = actsz > bufsz ? bufsz : actsz;
	if (reqsz == 0) {
		*pos = ramdisk_size;
		return 0;
	}
	copysz = reqsz - copy_from_user (ramdisk + *pos, buf, reqsz);
	*pos += copysz;
	return copysz;
}

/**
 * \par Function:
 * p3ramdisk_lseek
 *
 * \par Description:
 * Handle a request to seek the position of the RAM disk cursor.  This is called
 * by the kernel when the user space application issues an lseek on the device.
 *
 * \par Inputs:
 * - file: The file structure of the RAM disk.
 * - offset: The location of the new current pointer,
 *   relative to the starting position.
 * - orig: Determines the requested starting position:
 *   - SEEK_SET: Start from the beginning of the RAM disk
 *   - SEEK_CUR: Start from the current RAM disk file pointer
 *   - SEEK_END: Start from the end of the RAM disk
 *
 * \par Outputs:
 * - loff_t: New location of the file pointer.
 *   - >=0: Location changed successfully
 *   - <0: There was an error, specified by errno * -1
 */

static inline loff_t p3ramdisk_lseek (struct file *file, loff_t offset, int pos)
{
	loff_t newpos;
	switch (pos) {
	case SEEK_SET:
		newpos = offset;
		break;
	case SEEK_CUR:
		newpos = file->f_pos + offset;
		break;
	case SEEK_END:
		newpos = ramdisk_size + offset;
		break;
	default:
		return -EINVAL;
	}
	newpos = newpos < ramdisk_size ? newpos : ramdisk_size;
	newpos = newpos >= 0 ? newpos : 0;
	file->f_pos = newpos;
	return newpos;
}

/**
 * \par Function:
 * p3ioctl
 *
 * \par Description:
 * Handle an ioctl command from a user space application for the P3 RAM disk.
 * There are two ioctl commands supported:
 * <ul>
 *   <li>_IOC_READ: This is a request from the user space application to
 *   read data from the device driver.</li>
 *   <li>_IOC_WRITE: This is a request from the user space application to
 *   write data to the device driver.</li>
 * </ul>
 *
 * \par Inputs:
 * - file: The file structure for the RAM disk.
 * - cmd: The ioctl command.
 * - arg: The ioctl argument.
 *
 * \par Outputs:
 * - long: Status
 *   - 0 = OK
 *   - <0: There was an error, specified by errno * -1
 */

static inline long
p3ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
	int rc, direction;
	int size;
	char *buffer = NULL;
	void __user *ioargp = (void __user *)arg;

p3errmsg(p3MSG_DEBUG, "Enter ioctl handler\n");
	/* Validate command */
	if (_IOC_TYPE (cmd) != P3IOC_TYPE) {
		sprintf(p3buf, "%s: Invalid ioctl type: %d\n", P3APP, _IOC_TYPE (cmd));
		p3errmsg(p3MSG_WARN, p3buf);
		rc = -EINVAL;
		goto out;
	}

	size = _IOC_SIZE (cmd);
	buffer = kmalloc ((size_t) size, GFP_ATOMIC);
	if (buffer == NULL) {
		sprintf(p3buf, "%s: Error allocating ioctl buffer\n", P3APP);
		p3errmsg(p3MSG_CRIT, p3buf);
		rc = -ENOMEM;
		goto out;
	}

	direction = _IOC_DIR (cmd);

	switch (direction) {

	case _IOC_WRITE:
p3errmsg(p3MSG_DEBUG, "Write ioctl command\n");
		if ((rc = copy_from_user (buffer, ioargp, size)) < 0)
			goto out;
		if (parse_p3data((unsigned char *) buffer, size) < 0)
			rc = -EINVAL;
			goto out;
		break;

	case _IOC_READ:
p3errmsg(p3MSG_DEBUG, "Read ioctl command\n");
		rc = copy_to_user (ioargp, buffer, size);
		break;

	default:
		sprintf(p3buf, "%s: Invalid ioctl direction:%d\n", P3APP, direction);
		p3errmsg(p3MSG_WARN, p3buf);
		rc = -EINVAL;
		goto out;
	}

out:
	if (buffer != NULL)
		kfree (buffer);
	return rc;
}

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>Invalid ioctl type: <i>type</i></b>
 * \par Description (WARN):
 * An ioctl command from a user space application issued an invalid
 * command.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 * <hr><b>Error allocating ioctl buffer</b>
 * \par Description (CRIT):
 * There was not enough system memory to allocate the ioctl buffer.
 * \par Response:
 * Troubleshoot the system memory problem.
 *
 * <hr><b>Invalid ioctl direction: <i>direction</i></b>
 * \par Description (WARN):
 * An ioctl command from a user space application issued an invalid
 * direction (in or out are valid).
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */

/**
 * \par Function:
 * p3mmap
 *
 * \par Description:
 * Initialize the P3 RAM disk.  This is called by the kernel when the
 * device driver is initialized.
 *
 * \par Inputs:
 * - file: The file structure for the RAM disk.
 * - vma: The memory area structure for the RAM disk.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0: There was an error, specified by errno * -1
 */

static int p3mmap (struct file *file, struct vm_area_struct *vma)
{
	unsigned long pfn;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long len = vma->vm_end - vma->vm_start;

	if (offset >= ramdisk_size)
		return -EINVAL;
	if (len > (ramdisk_size - offset))
		return -EINVAL;

	/*    pfn = page_to_pfn (virt_to_page (ramdisk + offset)); */
	pfn = virt_to_phys (ramdisk + offset) >> PAGE_SHIFT;

	if (remap_pfn_range (vma, vma->vm_start, pfn, len, vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

/**
 * \par Function:
 * p3pkt_intercept
 *
 * \par Description:
 * Handle a packet being sent from a user space application.  This
 * is the common function for callbacks from kernel hooks.
 *
 * The packet_handler function is called to process the packet.  The
 * argument passed to this function is a p3packet structure, which points
 * to the packet data, including the IP and any following headers, in an
 * unsigned character array.  Therefore, this function must determine how
 * to provide that for the specific operating system being used.
 *
 * The return values are described in the packet_handler function and
 * are handled appropriately for the operating system.
 *
 * \par Inputs:
 * - hknum: Hook number.
 * - skb: Socket buffer structure.
 * - okfn: The function to be called to complete packet handling
 *
 * \par Outputs:
 * - int: Kernel stack instruction:
 *   - NF_ACCEPT: Process the packet normally.
 *   - NF_DROP: Drop the packet.
 *   - NF_STOLEN: The packet will be handled by this function.
 *   - NF_QUEUE: Queue packet for userspace.
 *   - NF_REPEAT: Call this hook function again.
 */

static unsigned int
p3pkt_intercept(unsigned int hknum, struct sk_buff *skb,
			const struct net_device *in, const struct net_device *out,
			int (*okfn)(struct sk_buff *))
{
	int i, stat, hd, tl;
	const struct iphdr *iph = ip_hdr(skb);
	struct sk_buff *skb2;
	p3packet pkt;
	p3netdata *netdata;


	memset(&pkt, 0, sizeof(p3packet));
	pkt.packet = (unsigned char *)iph;
	pkt.flag = skb->len;

	if ((stat = packet_handler(&pkt, (void *) skb)) < 0) {
p3errmsg(p3MSG_DEBUG, "Kernel intercept: packet error\n");
		return NF_DROP;
	// Control packet handled by P3 processing
	} else if (stat & p3PKTS_RAWSOCK) {
p3errmsg(p3MSG_DEBUG, "Kernel intercept: set Raw socket\n");
		if (pkt.work != NULL)
			p3free(pkt.work);
		kfree_skb(skb);
		netdata = (p3netdata *) pkt.net->netdata;
		netdata->okfn = okfn;
		return NF_STOLEN;
	// Packet generated by P3 not returned to stack
	} else if (stat & p3PKTS_CONTROL) {
p3errmsg(p3MSG_DEBUG, "Kernel intercept: drop P3 control packet\n");
		if (pkt.work != NULL)
			p3free(pkt.work);
		return NF_DROP;
	// Intercepted packet
	} else if (stat & (p3PKTS_ADDHDR | p3PKTS_RMVHDR)) {
p3errmsg(p3MSG_DEBUG, "Kernel intercept: handle P3 data packet\n");
sprintf(p3buf, "SKB: %p, SK %p, DST %x, DEV %p\n",
	skb, skb->sk, skb->p3SKB_DST, skb->dev);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  ML %d, DL %d, PL %d, Pr %d, Sz %d, Us %d\n",
	skb->mac_len, skb->data_len, skb->len,
	skb->protocol, skb->truesize, skb->users);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  DT %p, MH %p, NH %p, TH %p\n",
	skb->data, skb->p3SKB_MACHDR, skb->p3SKB_NETHDR, skb->p3SKB_TRANSHDR);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  HD %p, Tail %p, End %p\n", skb->head, skb->tail, skb->end);
p3errmsg(p3MSG_DEBUG, p3buf);
		if (pkt.packet == NULL) {
			if (pkt.work != NULL)
				p3free(pkt.work);
			sprintf(p3buf, "%s: Modified packet is NULL\n", P3APP);
			p3errmsg(p3MSG_DEBUG, p3buf);
			return NF_DROP;
		}
		if (stat & p3PKTS_ADDHDR) {
			i = (pkt.flag & p3PKT_SIZE) - skb->len;
			if (i > 0) {
				hd = skb_headroom(skb);
				tl = skb_tailroom(skb) + i;
				if ((skb2 = skb_copy_expand(skb, hd, tl, GFP_ATOMIC))
						== NULL) {
					sprintf(p3buf, "%s: Modified packet is too large\n", P3APP);
					p3errmsg(p3MSG_DEBUG, p3buf);
					if (pkt.work != NULL)
						p3free(pkt.work);
					return NF_DROP;
				}
				stat |= p3PKTS_NEW;
				skb2->sk = skb->sk;
#if p3LINUXVER >= 2624
				skb2->hdr_len = skb->hdr_len;
#endif
				kfree_skb(skb);
				skb = skb2;
				skb->ip_summed = CHECKSUM_NONE;
			}
		}
		// Copy new packet data
		skb->len = pkt.flag & p3PKT_SIZE;
		memcpy(skb->data, pkt.packet, skb->len);
		skb_set_tail_pointer(skb, skb->len);
#if p3LINUXVER >= 2624
		if (skb->hdr_len) {
			skb->hdr_len = skb_headroom(skb) + skb->len;
		}
#endif
		// TODO: Add support for IPv6 in Linux intercept handler
		i = (skb->data[0] & 0xf) << 2;
		skb_set_transport_header(skb, i);
		// TODO: Handle checksum better
		if (stat & p3PKTS_CHKSUM)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		if (pkt.work != NULL)
			p3free(pkt.work);
		if (pkt.flag & p3PKT_DSSUB) {
			if (p3net_utils(p3SET_FORWARD, (void *)skb, (void *)&pkt) < 0) {
				sprintf(p3buf, "%s: System network utility failed: %d\n",
						P3APP, p3SET_FORWARD);
				p3errmsg(p3MSG_ERR, p3buf);
				return NF_DROP;
			}
		}
		netdata = (p3netdata *) pkt.net->netdata;
		if (stat & p3PKTS_ADDHDR) {
			if (skb->sk == NULL)
				skb->sk = netdata->p3sk;
			if (!skb->p3SKB_DST) {
				p3SKB_DST_SET(skb, netdata->p3dst);
				dst_clone(netdata->p3dst);
			}
		}
		if (skb->dev == NULL)
			skb->dev = netdata->p3ndev;
sprintf(p3buf, "New SKB: %p, SK %p, DST %x\n",
	skb, skb->sk, skb->p3SKB_DST);
p3errmsg(p3MSG_DEBUG, p3buf);
if (skb->dev) {
  sprintf(p3buf, "  Dev %p Name %s\n", skb->dev, skb->dev->name);
  p3errmsg(p3MSG_DEBUG, p3buf);
} else {
  p3errmsg(p3MSG_DEBUG, "Dev NULL\n");
}
sprintf(p3buf, "  ML %d, DL %d, PL %d, Pr %d, Sz %d, Us %d\n",
	skb->mac_len, skb->data_len, skb->len,
	skb->protocol, skb->truesize, skb->users);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  DT %p, MH %p, NH %p, TH %p\n",
	skb->data, skb->p3SKB_MACHDR, skb->p3SKB_NETHDR, skb->p3SKB_TRANSHDR);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  HD %p, Tail %p, End %p\n", skb->head, skb->tail, skb->end);
p3errmsg(p3MSG_DEBUG, p3buf);
		// Packet from local host
		if ((stat & p3PKTS_NEW) && skb->sk != NULL)
			skb_set_owner_w(skb, skb->sk);
		// Forward packet to stack
		if (pkt.flag & p3PKT_DSSUB) {
p3errmsg(p3MSG_DEBUG, "Dest is subnet\n");
			if (skb->sk == NULL)
				skb->sk = netdata->p3sk;
			// Get correct destination
			if (p3ROUTE_HARDER(skb) != 0) {
p3errmsg(p3MSG_DEBUG, "Error in route lookup\n");
				return NF_DROP;
			}
if (skb->p3SKB_DST) {
  sprintf(p3buf, "New Dst %p Dev %p Name %s\n",
	p3SKB_DST_GET(skb), p3SKB_DST_GET(skb)->dev, p3SKB_DST_GET(skb)->dev->name);
  p3errmsg(p3MSG_DEBUG, p3buf);
}
			if (netdata->okfn(skb)) {
				sprintf(p3buf, "%s: Failed to queue forwarded data packet\n",
						P3APP);
				p3errmsg(p3MSG_DEBUG, p3buf);
			}
sprintf(p3buf, "Exit local forwarded packet intercept (%p)\n", netdata->okfn);
p3errmsg(p3MSG_DEBUG, p3buf);
			return NF_STOLEN;
		} else if (okfn(skb)) {
			sprintf(p3buf, "%s: Failed to queue data packet\n", P3APP);
			p3errmsg(p3MSG_DEBUG, p3buf);
// Freed in okfn???		return NF_DROP;
		}
sprintf(p3buf, "Exit packet intercept (%p)\n", skb);
p3errmsg(p3MSG_DEBUG, p3buf);
		return NF_STOLEN;
	}
	return NF_ACCEPT;
} /* end p3pkt_intercept */

/**
 * \par Function:
 * p3pkt_intercept_local
 *
 * \par Description:
 * Receive a packet captured for the kernel hook, NF_INET_LOCAL_OUT.
 *
 * \par Inputs:
 * - hknum: Hook number.
 * - SKBP: Socket buffer structure.
 * - in: Input network device.
 * - out: Output network device.
 * - okfn: The function to be called to complete packet handling
 *
 * \par Outputs:
 * - int: Return value from common packet handler, p3pkt_intr.
 */

static unsigned int
p3pkt_intercept_local(unsigned int hknum, struct sk_buff *SKBP,
			const struct net_device *in, const struct net_device *out,
			int (*okfn)(struct sk_buff *))
{
	return p3pkt_intercept(hknum, SKBP, in, out, okfn);
}

/**
 * \par Function:
 * p3pkt_intercept_pre
 *
 * \par Description:
 * Receive a packet captured for the kernel hook, NF_INET_PRE_ROUTING.
 *
 * \par Inputs:
 * - hknum: Hook number.
 * - SKBP: Socket buffer structure.
 * - in: Input network device.
 * - out: Output network device.
 * - okfn: The function to be called to complete packet handling
 *
 * \par Outputs:
 * - int: Return value from common packet handler, p3pkt_intr.
 */

static unsigned int
p3pkt_intercept_pre(unsigned int hknum, struct sk_buff *SKBP,
			const struct net_device *in, const struct net_device *out,
			int (*okfn)(struct sk_buff *))
{
	return p3pkt_intercept(hknum, SKBP, in, out, okfn);
}

/**
 * \par Function:
 * p3pkt_intercept_forward
 *
 * \par Description:
 * Receive a packet captured for the kernel hook, NF_INET_PRE_ROUTING.
 *
 * \par Inputs:
 * - hknum: Hook number.
 * - SKBP: Socket buffer structure.
 * - in: Input network device.
 * - out: Output network device.
 * - okfn: The function to be called to complete packet handling
 *
 * \par Outputs:
 * - int: Return value from common packet handler, p3pkt_intr.
 */

static unsigned int
p3pkt_intercept_forward(unsigned int hknum, struct sk_buff *SKBP,
			const struct net_device *in, const struct net_device *out,
			int (*okfn)(struct sk_buff *))
{
	int i, stat, hd, tl;
	const struct iphdr *iph = ip_hdr(SKBP);
	struct sk_buff *skb2;
	p3packet pkt;
	p3netdata *netdata;

	memset(&pkt, 0, sizeof(p3packet));
	pkt.packet = (unsigned char *)iph;
	// Forward flag is XOR'ed in host lookup
	// Setting it here allows packet handler to do encryption
	pkt.flag = SKBP->len | p3PKT_SRSUB;
	if ((stat = packet_handler(&pkt, (void *) SKBP)) < 0) {
p3errmsg(p3MSG_DEBUG, "Kernel intercept: forwarded packet error\n");
		return NF_DROP;
	// Control packet handled by P3 processing
	} else if (stat & (p3PKTS_ADDHDR | p3PKTS_RMVHDR)) {
p3errmsg(p3MSG_DEBUG, "Kernel intercept: handle forwarded P3 data packet\n");
sprintf(p3buf, "SKB: %p, SK %p, DST %x, DEV %p\n",
	SKBP, SKBP->sk, SKBP->p3SKB_DST, SKBP->dev);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  ML %d, DL %d, PL %d, Pr %d, Sz %d, Us %d\n",
	SKBP->mac_len, SKBP->data_len, SKBP->len,
	SKBP->protocol, SKBP->truesize, SKBP->users);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  DT %p, MH %p, NH %p, TH %p\n",
	SKBP->data, SKBP->p3SKB_MACHDR, SKBP->p3SKB_NETHDR, SKBP->p3SKB_TRANSHDR);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  HD %p, Tail %p, End %p\n", SKBP->head, SKBP->tail, SKBP->end);
p3errmsg(p3MSG_DEBUG, p3buf);
		if (pkt.packet == NULL) {
			if (pkt.work != NULL)
				p3free(pkt.work);
			sprintf(p3buf, "%s: Modified packet is NULL\n", P3APP);
			p3errmsg(p3MSG_DEBUG, p3buf);
			return NF_DROP;
		}
		if (stat & p3PKTS_ADDHDR) {
			i = (pkt.flag & p3PKT_SIZE) - SKBP->len;
			if (i > 0) {
				hd = skb_headroom(SKBP);
				tl = skb_tailroom(SKBP) + i;
				if ((skb2 = skb_copy_expand(SKBP, hd, tl, GFP_ATOMIC))
						== NULL) {
					sprintf(p3buf, "%s: Modified packet is too large\n", P3APP);
					p3errmsg(p3MSG_DEBUG, p3buf);
					if (pkt.work != NULL)
						p3free(pkt.work);
					return NF_DROP;
				}
				stat |= p3PKTS_NEW;
				skb2->sk = SKBP->sk;
#if p3LINUXVER >= 2624
				skb2->hdr_len = SKBP->hdr_len;
#endif
				kfree_skb(SKBP);
				SKBP = skb2;
				SKBP->ip_summed = CHECKSUM_NONE;
			}
		}
		// Copy new packet data
		SKBP->len = pkt.flag & p3PKT_SIZE;
		memcpy(SKBP->data, pkt.packet, SKBP->len);
		skb_set_tail_pointer(SKBP, SKBP->len);
#if p3LINUXVER >= 2624
		if (SKBP->hdr_len) {
			SKBP->hdr_len = skb_headroom(SKBP) + SKBP->len;
		}
#endif
		// TODO: Add support for IPv6 in Linux intercept handler
		i = (SKBP->data[0] & 0xf) << 2;
		skb_set_transport_header(SKBP, i);
		// TODO: Handle checksum better
		if (stat & p3PKTS_CHKSUM)
			SKBP->ip_summed = CHECKSUM_UNNECESSARY;
		if (pkt.work != NULL)
			p3free(pkt.work);
		// Get correct destination
		netdata = (p3netdata *) pkt.net->netdata;
		if (SKBP->sk == NULL)
			SKBP->sk = netdata->p3sk;
		if (!SKBP->p3SKB_DST) {
			p3SKB_DST_SET(SKBP, netdata->p3dst);
			dst_clone(netdata->p3dst);
		}
		if (SKBP->dev == NULL)
			SKBP->dev = netdata->p3ndev;
		if (p3ROUTE_HARDER(SKBP) != 0) {
p3errmsg(p3MSG_DEBUG, "Error in route lookup\n");
			return NF_DROP;
		}
sprintf(p3buf, "New SKB: %p, SK %p, DST %x\n",
	SKBP, SKBP->sk, SKBP->p3SKB_DST);
p3errmsg(p3MSG_DEBUG, p3buf);
if (SKBP->dev) {
  sprintf(p3buf, "  Dev %p Name %s\n", SKBP->dev, SKBP->dev->name);
  p3errmsg(p3MSG_DEBUG, p3buf);
} else {
  p3errmsg(p3MSG_DEBUG, "Dev NULL\n");
}
sprintf(p3buf, "  ML %d, DL %d, PL %d, Pr %d, Sz %d, Us %d\n",
	SKBP->mac_len, SKBP->data_len, SKBP->len,
	SKBP->protocol, SKBP->truesize, SKBP->users);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  DT %p, MH %p, NH %p, TH %p\n",
	SKBP->data, SKBP->p3SKB_MACHDR, SKBP->p3SKB_NETHDR, SKBP->p3SKB_TRANSHDR);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  HD %p, Tail %p, End %p\n", SKBP->head, SKBP->tail, SKBP->end);
p3errmsg(p3MSG_DEBUG, p3buf);
		// Packet from local host
		if ((stat & p3PKTS_NEW) && SKBP->sk != NULL)
			skb_set_owner_w(SKBP, SKBP->sk);
		// Forward packet to stack
		if (okfn(SKBP)) {
			sprintf(p3buf, "%s: Failed to queue data packet\n", P3APP);
			p3errmsg(p3MSG_DEBUG, p3buf);
// Freed in okfn???		return NF_DROP;
		}
sprintf(p3buf, "Exit forwarded packet intercept (%p)\n", SKBP);
p3errmsg(p3MSG_DEBUG, p3buf);
		return NF_STOLEN;
	}
	return NF_ACCEPT;
} /* end p3pkt_intercept_forward */

/**
 * \par Function:
 * p3pkt_intercept_final
 *
 * \par Description:
 * Receive a packet captured for the kernel hook, NF_INET_PRE_ROUTING.
 *
 * \par Inputs:
 * - hknum: Hook number.
 * - skb: Socket buffer structure.
 * - in: Input network device.
 * - out: Output network device.
 * - okfn: The function to be called to complete packet handling
 *
 * \par Outputs:
 * - int: Return value from common packet handler, p3pkt_intr.
 */

static unsigned int
p3pkt_intercept_final(unsigned int hknum, struct sk_buff *SKBP,
			const struct net_device *in, const struct net_device *out,
			int (*okfn)(struct sk_buff *))
{
//	return p3pkt_intercept(hknum, SKBP, in, out, okfn);
	return NF_ACCEPT;
}

/**
 * \par Function:
 * p3send_packet
 *
 * \par Description:
 * Send a packet directly from the P3 kernel module.  The entire
 * packet must have been built except for the IP checksum, which
 * is calculated here.
 *
 * \par Inputs:
 * - p3pkt: The packet structure
 *
 * \par Outputs:
 * - int: Return value from common packet handler, p3pkt_intr.
 */

int p3send_packet(void *p3pkt)
{
	int len, stat = 0;
	struct sk_buff *skb;
	struct iphdr *iph;
	p3packet *pkt = (p3packet *) p3pkt;
	p3netdata *netdata = (p3netdata *) pkt->host->net->netdata;
//	struct kiocb iocb;
//	init_sync_kiocb(&iocb, NULL);
#if p3LINUXVER < 2624
	struct timeval skbts;
#endif

	if (netdata == NULL) {
		stat = -1;
		goto out;
	}

	if (netdata->p3proto == NULL || netdata->p3ndev == NULL) {
		stat = -1;
		goto out;
	} else if (netdata->p3sk == NULL) {
#if p3LINUXVER < 2624
		if ((netdata->p3sk = sk_alloc(PF_INET, GFP_ATOMIC,
				netdata->p3proto, 1)) == NULL) {
			stat = -1;
			goto out;
		}
#else
	
		if (netdata->p3knet == NULL ||
				(netdata->p3sk = sk_alloc(netdata->p3knet,
				PF_INET, GFP_ATOMIC, netdata->p3proto)) == NULL) {
			stat = -1;
			goto out;
		}
#endif
	}

	// Allocate skb with data field on 32 byte boundary
	len = ((pkt->flag & p3PKT_SIZE) + 0x1f) & ~0x1f;
	if (!(netdata->p3ndev->flags & IFF_UP)) {
p3errmsg(p3MSG_DEBUG, "Network is down\n");
		stat = -ENETDOWN;
		goto out;
	}
	if (len > netdata->p3ndev->mtu + netdata->p3ndev->hard_header_len) {
		stat = -EMSGSIZE;
		goto out;
	}
sprintf(p3buf, "Ctl pkt SKB: SK %p, Len %d\n", netdata->p3sk, len);
p3errmsg(p3MSG_DEBUG, p3buf);
	if ((skb = alloc_skb(len + LL_RESERVED_SPACE(netdata->p3ndev),
				GFP_ATOMIC)) == NULL) {
		stat = -ENOBUFS;
		goto out;
	}
	skb_set_owner_w(skb, netdata->p3sk);

sprintf(p3buf, "Netdata: Prot %x Sk %p Dst %p Dev %p\n",
	netdata->p3prot, netdata->p3sk, netdata->p3dst, netdata->p3ndev);
p3errmsg(p3MSG_DEBUG, p3buf);
	skb_reserve(skb, LL_RESERVED_SPACE(netdata->p3ndev));
	skb_reset_network_header(skb);
	/* Try to align data correctly */
	memcpy(skb_put(skb, len), pkt->packet, (pkt->flag & p3PKT_SIZE));
	iph = (struct iphdr *) skb->data;
	ip_fast_csum(skb_network_header(skb), iph->ihl);
	skb_set_transport_header(skb, p3SESSION_HDR4);
	skb->protocol = htons(netdata->p3prot);
#if p3LINUXVER < 2624
	skb_get_timestamp(skb, &skbts);
#else
	skb->tstamp = ktime_get_real();
#endif
	skb->ip_summed = CHECKSUM_NONE;
	skb->pkt_type = PACKET_OUTGOING;
	skb->dev = netdata->p3ndev;
	p3SKB_DST_SET(skb, netdata->p3dst);
	dst_clone(netdata->p3dst);
	skb->priority = netdata->p3sk->sk_priority;
	if (!(pkt->flag & p3PKT_CFWD)) {
sprintf(p3buf, "MAC: Loc %2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x Rem %2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x\n",
	netdata->p3locadr[0], netdata->p3locadr[1], netdata->p3locadr[2],
	netdata->p3locadr[3], netdata->p3locadr[4], netdata->p3locadr[5],
	netdata->p3remadr[0], netdata->p3remadr[1], netdata->p3remadr[2],
	netdata->p3remadr[3], netdata->p3remadr[4], netdata->p3remadr[5]);
p3errmsg(p3MSG_DEBUG, p3buf);
		p3MAC_HEADER_SET(skb)(skb, netdata->p3ndev, netdata->p3ptype,
				netdata->p3remadr, netdata->p3locadr, skb->len);
//	dev_hard_header(skb, netdata->p3ndev, netdata->p3ptype,
//					netdata->p3remadr, netdata->p3locadr, skb->len);
		if (skb_mac_header(skb) == NULL) {
			skb_reset_mac_header(skb);
		}
#if p3LINUXVER < 2624
		skb->mac_len = skb->nh.raw - skb->mac.raw;
#else
		skb->mac_len = skb->p3SKB_NETHDR - skb->p3SKB_MACHDR;
#endif
	}

sprintf(p3buf, "Control SKB %p: SK %p PL %d ML %d\n",
	skb, skb->sk, skb->len, skb->mac_len);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  DST %x, DEV %p, Name %s\n",
	skb->p3SKB_DST, skb->dev, skb->dev->name);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  HD %p, Tail %p, End %p\n", skb->head, skb->tail, skb->end);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  DT %p, MH %p, NH %p, TH %p\n",
	skb->data, skb->p3SKB_MACHDR, skb->p3SKB_NETHDR, skb->p3SKB_TRANSHDR);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  Dev adr: ");
for (len=0; len < 6; len++) {
	sprintf(p3buf, "%s %2.2x", p3buf, skb->dev->dev_addr[len]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);

sprintf(p3buf, "  MAC HDR:");
for (len=0; len < 64; len++) {
	if (!(len & 0xf))
		sprintf(p3buf, "%s\n  ", p3buf);
	else if (!(len & 0x3))
		sprintf(p3buf, "%s ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, skb->data[len]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
if (pkt->flag & p3PKT_CFWD)
	p3errmsg(p3MSG_DEBUG, "Call okfn\n");
else
	p3errmsg(p3MSG_DEBUG, "Call dev_queue_xmit\n");

	if ((pkt->flag & p3PKT_CFWD) && (stat = netdata->okfn(skb)) < 0) {
		sprintf(p3buf, "%s: Failed to queue control packet\n", P3APP);
		p3errmsg(p3MSG_DEBUG, p3buf);
	} else if (!(pkt->flag & p3PKT_CFWD) &&
			(stat = dev_queue_xmit(skb)) < 0) {
		sprintf(p3buf, "%s: Failed to queue control packet\n", P3APP);
		p3errmsg(p3MSG_DEBUG, p3buf);
	}

out:
	if (stat) {
sprintf(p3buf, "  Error status = %d\n", stat);
p3errmsg(p3MSG_DEBUG, p3buf);
	}
	return (stat);
} /* end p3send_packet */

/**
 * \par Function:
 * p3net_utils
 *
 * \par Description:
 * Handle network utility functions.  These include:
 * - Set a TCP checksum
 * - Get the MTU size for an interface
 *
 * \par Inputs:
 * - type: Utility function type:
 *   - p3TCP_CHECK
 *   - p3TCP_CHECK_ADD
 *   - p3GET_MTU
 *   - p3SET_DEVIN
 *   - p3SET_DEVOUT
 *   - p3SET_RAW
 * - p3skb: Socket buffer structure which is cast to the platform
 *   specific structure.
 * - p3pkt: The packet structure, which is cast to a p3packet struture,
 *   may be used to pass or return data.
 *
 * \par Outputs:
 * - Status
 */

int p3net_utils(int type, void *p3skb, void *p3pkt)
{
	int i, stat = 0;
	short int s1, s2;
	unsigned char *hdr;
	p3packet *pkt = (p3packet *) p3pkt;
	p3netdata *netdata;
	struct sk_buff *skb = (struct sk_buff *) p3skb;
	struct iphdr *iph;
	struct tcphdr *th;

	switch(type) {
	case p3TCP_CHECK:
		if (pkt->work == NULL) {
			p3errmsg(p3MSG_DEBUG, "p3net_utils: No packet work data\n");
			stat = -1;
			goto out;
		}
		th = pkt->work->tcph;
		s1 = pkt->netdata & 0xffff;
		s2 = (pkt->netdata >> 16) & 0xffff;
#if p3LINUXVER < 2624
		iph = (struct iphdr *) pkt->packet;
		th->check = 0;
		i = th->doff << 2;
		th->check = tcp_v4_check(th, i, iph->saddr, iph->daddr,
					csum_partial((char *)th, th->doff << 2, skb->csum));
#else
		inet_proto_csum_replace2(&th->check, skb, htons(s1), htons(s2), 0);
#endif
		break;

	case p3TCP_CHECK_ADD:
		if (skb->sk == NULL) {
			stat = -1;
			goto out;
		}
		// Packet: packet = added data, netdata = new TCP header size
		if (skb_tailroom(skb) < 6) {
			stat = -1;
			goto out;
		}
		skb->len = pkt->flag & p3PKT_SIZE;
		memcpy(skb->data, pkt->packet, skb->len);
		skb_set_tail_pointer(skb, skb->len);
		iph = ip_hdr(skb);
		iph->check = 0;
		ip_fast_csum(skb_network_header(skb), iph->ihl);
		th = tcp_hdr(skb);
		th->check = 0;
// TODO: Use skb_checksum_complete(skb)???
//		skb->ip_summed = CHECKSUM_NONE;
		th->check = __skb_checksum_complete(skb);
//		if (skb->ip_summed == CHECKSUM_PARTIAL) {
//			th->check = ~tcp_v4_check(skb->len, iph->saddr, iph->daddr, 0);
//			skb->csum_start = skb_transport_header(skb) - skb->head;
//			skb->csum_offset = offsetof(struct tcphdr, check);
//		} else {
//			th->check = tcp_v4_check(skb->len, iph->saddr, iph->daddr,
//						csum_partial(th, th->doff << 2, skb->csum));
//		}
		break;

	case p3GET_MTU:
		if (skb->dev == NULL) {
// TODO: Get legitimate MTU
			pkt->netdata = 1500;
		} else
			pkt->netdata = skb->dev->mtu;
		break;

	// Set skb information for packets forwarded to local subnet
	case p3SET_FORWARD:
		if ((netdata = (p3netdata *) pkt->net->netdata) == NULL) {
			stat = -1;
			goto out;
		}
p3errmsg(p3MSG_DEBUG, "Set Forward\n");
		dst_clone(netdata->p3dst);
		p3SKB_DST_SET(skb, netdata->p3dst);
		skb->dev = netdata->p3ndev;
	break;

	// Get session information for control packets
	case p3SET_DEVIN:
		if (pkt->net->netdata == NULL && (pkt->net->netdata = (void *)
				p3calloc(sizeof(p3netdata))) == NULL) {
			stat = -1;
			goto out;
		}
		netdata = (p3netdata *) pkt->net->netdata;
		netdata->p3ndev = skb->dev;
		netdata->p3prot = ntohs(skb->protocol);
		hdr = (unsigned char *) skb_mac_header(skb);
		memcpy(netdata->p3locadr, hdr, 6);
		memcpy(netdata->p3remadr, &hdr[6], 6);
		i = hdr[12];
		i <<= 8;
		i |= hdr[13];
		if (i < 0x600)
			netdata->p3ptype = ETH_P_802_3;
		else
			netdata->p3ptype = netdata->p3prot;
sprintf(p3buf, "Set dev in: %p Proto %p Dev %p\n",
	netdata, netdata->p3proto, netdata->p3ndev);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  Prot %d Type %d\n", netdata->p3prot, netdata->p3ptype);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  Loc %2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x Rem %2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x\n",
	netdata->p3locadr[0], netdata->p3locadr[1], netdata->p3locadr[2],
	netdata->p3locadr[3], netdata->p3locadr[4], netdata->p3locadr[5],
	netdata->p3remadr[0], netdata->p3remadr[1], netdata->p3remadr[2],
	netdata->p3remadr[3], netdata->p3remadr[4], netdata->p3remadr[5]);
p3errmsg(p3MSG_DEBUG, p3buf);
	break;

	case p3SET_DEVOUT:
		if (pkt->net->netdata == NULL && (pkt->net->netdata = (void *)
				p3calloc(sizeof(p3netdata))) == NULL) {
			stat = -1;
			goto out;
		}
		netdata = (p3netdata *) pkt->net->netdata;
		// Get session information for control packets
		if (skb->sk != NULL) {
#if p3LINUXVER >= 2624
			netdata->p3knet = sock_net(skb->sk);
#endif
			netdata->p3proto = skb->sk->sk_prot;
		}
		if (skb->dev == NULL)
			skb->dev = netdata->p3ndev;
		if (!p3SKB_DST_GET(skb))
			p3SKB_DST_SET(skb, netdata->p3dst);
#if p3LINUXVER < 2624
sprintf(p3buf, "Set dev out: %p Proto %p Dev %p\n",
	netdata, netdata->p3proto, netdata->p3ndev);
#else
sprintf(p3buf, "Set dev out: %p Proto %p Net %p Dev %p\n",
	netdata, netdata->p3proto, netdata->p3knet, netdata->p3ndev);
#endif
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  Prot %d Type %d\n", netdata->p3prot, netdata->p3ptype);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  Loc %2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x Rem %2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x\n",
	netdata->p3locadr[0], netdata->p3locadr[1], netdata->p3locadr[2],
	netdata->p3locadr[3], netdata->p3locadr[4], netdata->p3locadr[5],
	netdata->p3remadr[0], netdata->p3remadr[1], netdata->p3remadr[2],
	netdata->p3remadr[3], netdata->p3remadr[4], netdata->p3remadr[5]);
p3errmsg(p3MSG_DEBUG, p3buf);
	break;

	case p3SET_RAW:
		if (pkt->net->netdata == NULL && (pkt->net->netdata = (void *)
				p3calloc(sizeof(p3netdata))) == NULL) {
			stat = -1;
			goto out;
		}
		netdata = (p3netdata *) pkt->net->netdata;
		netdata->p3sk = skb->sk;
		netdata->p3dst = p3SKB_DST_GET(skb);
		dst_clone(netdata->p3dst);
		if (netdata->p3dst != NULL) {
			netdata->p3ndev = netdata->p3dst->dev;
sprintf(p3buf, "Set raw: SK %p Dst %p Dev %p Name %s\n",
	netdata->p3sk, netdata->p3dst, netdata->p3ndev, netdata->p3ndev->name);
p3errmsg(p3MSG_DEBUG, p3buf);
		} else {
sprintf(p3buf, "Set raw: SK %p Dst %p Dev %p\n",
	netdata->p3sk, netdata->p3dst, netdata->p3ndev);
p3errmsg(p3MSG_DEBUG, p3buf);
		}
	break;

	default:
		break;
	}

out:
	return(stat);
}


static const struct file_operations ramdisk_fops = {
	.open  = p3ramdisk_open,
	.read  = p3ramdisk_read,
	.write = p3ramdisk_write,
	.mmap  = p3mmap,
	.unlocked_ioctl = p3ioctl,
	.owner = THIS_MODULE
};

static struct nf_hook_ops netmod_reg[] __read_mostly = {
	{ .pf       = PF_INET,
	  .hook     = p3pkt_intercept_pre,
	  .hooknum  = p3PRE_ROUTING,
	  .priority = NF_IP_PRI_FIRST,
	  .owner    = THIS_MODULE },
	{ .pf       = PF_INET,
	  .hook     = p3pkt_intercept_local,
	  .hooknum  = p3LOCAL_OUT,
	  .priority = NF_IP_PRI_FIRST,
	  .owner    = THIS_MODULE },
	{ .pf       = PF_INET,
	  .hook     = p3pkt_intercept_forward,
	  .hooknum  = p3FORWARD,
	  .priority = NF_IP_PRI_FIRST,
	  .owner    = THIS_MODULE },
//	{ .pf       = PF_INET,
//	  .hook     = p3pkt_intercept_final,
//	  .hooknum  = p3LOCAL_IN,
//	  .priority = NF_IP_PRI_FIRST,
//	  .owner    = THIS_MODULE },
//	{ .pf       = PF_INET,
//	  .hook     = p3pkt_intercept_final,
//	  .hooknum  = p3POST_ROUTING,
//	  .priority = NF_IP_PRI_FIRST,
//	  .owner    = THIS_MODULE }
};

/**
 * \par Function:
 * P3INIT (p3primary_init, p3secondary_init, p3primaryplus_init)
 *
 * \par Description:
 * Initialize the P3 Linux kernel module.  This includes:
 * <ul>
 *   <li>Initialize the RAM disk device driver</li>
 *   <li>Initialize the packet intercept handlers</li>
 * </ul>
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

#ifdef _p3_PRIMARY
static int __init p3primary_init (void)
#else
  #ifdef _p3_SECONDARY
static int __init p3secondary_init (void)
  #else
static int __init p3primaryplus_init (void)
  #endif
#endif
{
	int stat = 0;

	if (alloc_chrdev_region (&ramdisk_region, 0, count, P3DEVNAME) < 0) {
		sprintf(p3buf, "%s: Error allocating character device region\n", P3APP);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}
	if (!(ramdisk_cdev = cdev_alloc ())) {
		sprintf(p3buf, "%s: Error allocating character device\n", P3APP);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -2;
		goto out;
	}
	cdev_init (ramdisk_cdev, &ramdisk_fops);

	if ((ramdisk = kmalloc (ramdisk_size, GFP_ATOMIC)) == NULL) {
		sprintf(p3buf, "%s: Error allocating ramdisk\n", P3APP);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -3;
		goto out;
	}

	if (cdev_add (ramdisk_cdev, ramdisk_region, count) < 0) {
		sprintf(p3buf, "%s: Error adding character device\n", P3APP);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -3;
		goto out;
	}

	ramdisk_class = class_create (THIS_MODULE, "ramdisk_class");
	if (IS_ERR(ramdisk_class)) {
		sprintf(p3buf, "%s: Error creating ramdisk class\n", P3APP);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -4;
		goto out;
	}
#if p3LINUXVER < 2624
	ramdisk_device = device_create (ramdisk_class, NULL, ramdisk_region,
							"%s", P3DEVNAME);
#else
	ramdisk_device = device_create (ramdisk_class, NULL, ramdisk_region,
							NULL, "%s", P3DEVNAME);
#endif
	if (IS_ERR(ramdisk_device)) {
		sprintf(p3buf, "%s: Error creating ramdisk device\n", P3APP);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -5;
		goto out;
	}

	if (nf_register_hooks(netmod_reg, ARRAY_SIZE(netmod_reg))) {
		sprintf(p3buf, "%s: Error registering netfilter hook\n", P3APP);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -6;
		goto out;
	}

sprintf(p3buf, "RAM disk %p\n", ramdisk);
p3errmsg(p3MSG_DEBUG, p3buf);

	// TODO: Start timer thread
#ifdef _p3_PRIMARY
	if (init_primary(ramdisk) < 0) {
		stat = -8;
		goto out;
	}
#endif
#ifdef _p3_SECONDARY
	if (init_secondary() < 0) {
		stat = -8;
		goto out;
	}
#endif
#ifdef _p3_PRIMARY_PLUS
	if (init_primaryplus() < 0) {
		stat = -8;
		goto out;
	}
#endif
	// Initialize P3 networking
	if (init_p3net() < 0) {
		stat = -8;
		goto out;
	}

	sprintf(p3buf, "%s: Initialization complete\n", P3APP);
	p3errmsg(p3MSG_NOTICE, p3buf);

out:
	if (stat < -6) {
		nf_unregister_hooks(netmod_reg, ARRAY_SIZE(netmod_reg));
	}
	if (stat < -5) {
		device_destroy (ramdisk_class, ramdisk_region);
	}
	if (stat < -4) {
		class_destroy (ramdisk_class);
	}
	if (stat < -3) {
		cdev_del (ramdisk_cdev);
	}
	if (stat < -2) {
		kfree (ramdisk);
	}
	if (stat < -1) {
		unregister_chrdev_region (ramdisk_region, count);
	}
	return stat;
}

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>Error allocating character device region</b>
 * \par Description (CRIT):
 * While initializing the P3 kernel module, the character device region
 * could not be allocated.
 * \par Response:
 * Troubleshoot the system memory problem.
 *
 * <hr><b>Error allocating character device</b>
 * \par Description (CRIT):
 * While initializing the P3 kernel module, the character device
 * could not be allocated.
 * \par Response:
 * Troubleshoot the system memory problem.
 *
 * <hr><b>Error allocating ramdisk</b>
 * \par Description (CRIT):
 * While initializing the P3 kernel module, the RAM disk space
 * could not be allocated.
 * \par Response:
 * Troubleshoot the system memory problem.
 *
 * <hr><b>Error adding character device</b>
 * \par Description (CRIT):
 * The character device providing access to the P3 RAM disk
 * could not be added to the system.
 * \par Response:
 * Troubleshoot the system device problem.
 *
 * <hr><b>Error creating ramdisk class</b>
 * \par Description (CRIT):
 * The P3 RAM disk class could not be added to the system.
 * \par Response:
 * Troubleshoot the system device problem.
 *
 * <hr><b>Error creating ramdisk device</b>
 * \par Description (CRIT):
 * The P3 RAM disk device could not be added to the system.
 * \par Response:
 * Troubleshoot the system device problem.
 *
 * <hr><b>Error registering netfilter hook</b>
 * \par Description (CRIT):
 * The P3 netfilter hook for intercepting packets could
 * not be added to the system.
 * \par Response:
 * Troubleshoot the system network problem.
 *
 * <hr><b>Initialization complete</b>
 * \par Description (NOTICE):
 * The P3 kernel module is successfully initialized.
 * \par Response:
 * No response required.
 *
 */

/**
 * \par Function:
 * P3EXIT (p3primary_exit, p3secondary_exit, p3primaryplus_exit)
 *
 * \par Description:
 * Cleanly exit the P3 Linux kernel module.  This includes:
 * <ul>
 *   <li>Close the packet intercept handlers</li>
 *   <li>Close and free the RAM disk device driver</li>
 * </ul>
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

#ifdef _p3_PRIMARY
static void __exit p3primary_exit (void)
#else
  #ifdef _p3_SECONDARY
static void __exit p3secondary_exit (void)
  #else
static void __exit p3primaryplus_exit (void)
  #endif
#endif
{
	// TODO: Shutdown timer thread
	// TODO: Release all dst structures
//	if (p3dst != NULL) {
//		dst_release(p3dst);
//	}
#ifdef _p3_PRIMARY
	kfree(primain);
	if (ipv4route != NULL)
		kfree(ipv4route);
	if (ipv6route != NULL)
		kfree(ipv6route);
#endif
#ifdef _p3_SECONDARY
	kfree(secmain);
#endif
#ifdef _p3_PRIMARYPLUS
	kfree(primain);
	if (ipv4route != NULL)
		kfree(ipv4route);
	if (ipv6route != NULL)
		kfree(ipv6route);
#endif
	nf_unregister_hooks(netmod_reg, ARRAY_SIZE(netmod_reg));
	device_destroy (ramdisk_class, ramdisk_region);
	class_destroy (ramdisk_class);
	if (ramdisk_cdev)
		cdev_del (ramdisk_cdev);
	unregister_chrdev_region (ramdisk_region, count);
	kfree (ramdisk);

	sprintf(p3buf, "%s: Exit\n", P3APP);
	p3errmsg(p3MSG_INFO, p3buf);
}

/**
 * Kernel Functions:
 * \par module_init
 *  Register the kernel module initialization routine with the Linux kernel.
 * \par module_exit
 *  Register the kernel module exit routine with the Linux kernel.
 */

#ifdef _p3_PRIMARY
module_init (p3primary_init);
module_exit (p3primary_exit);
#else
  #ifdef _p3_SECONDARY
module_init (p3secondary_init);
module_exit (p3secondary_exit);
  #else
module_init (p3primaryplus_init);
module_exit (p3primaryplus_exit);
  #endif
#endif
