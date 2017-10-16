# rtacl::db - An R-tree based ACL


## Introduction

**rtacl::db** is an ACL (Access Control List) C++ class using
R-tree. **rtacl::db** supports both IPv4 and IPv6 address
ranges as well as TCP/UDP port range, IP protocol range, and
DSCP range although IP protocol and DSCP may not need range
matching.


## Requirements

* **rtacl::db** uses **boost::geometry::index::rtree**. It is
  necessary to install [boost C++ libraries](http://www.boost.org/).
  Version 1.63 (or newer) is preferable.
* *local_types.h* must be stored in the include path as well as
  *rtacl.hpp*

## Performance

* CPU: Intel(R) Core(TM) i3-4160T CPU @ 3.10GHz (cache size: 3072 KB)
* Memory: 4GB
* Compiler options: -O3 -DNDEBUG


### Search (matching)

830ns/search with 1 million IPv4 ACL entries.


### Search (un-matching)

780ns/search with 1 million IPv4 ACL entries.


### Insertion

3.27 us/insertion in average while inserting 1 million IPv4 ACL
entries.


### Deletion

1.25 us/insertion in average while deleting 1 million IPv4 ACL
entries.


## Data Structures

* **rtacl::ipv4a**: IPv4 address to be used inside
  **rtacl::db**, which is a *typedef* of **int64_t**. This must
  be in the host byte order.
* **rtacl::ipv6a**: IPv6 address to be used inside
  **rtacl::db**, which is a *typedef* of
  **boost::multiprecision::int256_t**. This must be in the
  host byte order.
* **rtacl::tuple<ADDR>**: ACL tuple (search key.) Template
  parameter **ADDR** must be either **rtacl::ipv4a** or
  **rtacl::ipv6a**. This is a *typedef* of
  **boost::geometry::point** (6 dimensional.) **Its order must
  be source IPv4/IPv6 address, destination IPv4/IPv6 address,
  source port number, destination port number, IP protocol,
  and DSCP**.
* **rtacl::range<ADDR>**: The ACL range defined by two
  **rtacl::tuple**s (minimum and maximum). Template parameter
  **ADDR** must be either **rtacl::ipv4a** or **rtacl::ipv6a**.
  This is a *typedef* of **boost::geometry::model::box**, i.e.,
  a multi-dimensional polygon defined by two multi-dimensional
  points.
* **rtacl::entry<ADDR>**: R-tree ACL entry. Template parameter
  **ADDR** must be either **rtacl::ipv4a** or
  **rtacl::ipv6a**. **rtacl::entry** is a *typedef* of
  **std::pair** comprising **rtacl::range** (first type) and
  the associated unique index (second type.) R-tree is an
  algorithm to index multi-dimensional polygons. Index can 
  be any integer as long as it is unique to the associated
  **rtacl::emtry**. However, it is typical for users to prepare
  their own ACL entry data structure associated 
  with **rtacl::entry** (see *unitTest.cpp*) so that index is
  usually the pointer of user-defined ACL entry associated with
  an **rtacl::entry**.
* **rtacl::result<ADDR>**: R-tree ACL search result. Template
  parameter **ADDR** must be either **rtacl::ipv4a** or
  **rtacl::ipv6a**. **rtacl::result<ADDR>** is a *typedef* of
  **std::vector<rtacl::entry<ADDR>>**;

* **rtacl::db<ADDR>**: R-tree ACL class. Template parameter
  **ADDR** must be either **rtacl::ipv4a** or **rtacl::ipv6a**.
  **rtacl::db** is not intrusive.


## rtacl::db<ADDR>

### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)

### Member Functions

```C++
template <class ADDR>
void rtacl::db::makeMin(const ADDR sa,
                        const ADDR da,
                        const ADDR sp,
                        const ADDR dp,
                        const ADDR proto,
                        const ADDR dscp,
                        tuple<ADDR>& result);
```

Makes the lower bound of an R-tree ACL entry

##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)

##### Input Parameters

* **sa**: Source IP address (IPv4 or IPv6 depending on **ADDR**)
* **da**: Destination IP address (IPv4 or IPv6 depending on **ADDR**)
* **sp**: Source port number
* **dp**: Destination port number
* **proto**: IP Protocol (TCP, UDP, etc.)
* **dscp**: DSCP

##### Output Parameters

* **result**: Lower bound R-tree ACL tuple representing the
  input parameters for insertion and deletion.


```C++
template <class ADDR>
void rtacl::db::makeMax(const ADDR sa,
                        const ADDR da,
                        const ADDR sp,
                        const ADDR dp,
                        const ADDR proto,
                        const ADDR dscp,
                        tuple<ADDR>& result);
```

Makes the upper bound of an R-tree ACL entry

##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)

##### Input Parameters

* **sa**: Source IP address (IPv4 or IPv6 depending on **ADDR**)
* **da**: Destination IP address (IPv4 or IPv6 depending on **ADDR**)
* **sp**: Source port number
* **dp**: Destination port number
* **proto**: IP Protocol (TCP, UDP, etc.)
* **dscp**: DSCP

##### Output Parameters

* **result**: Upper bound R-tree ACL tuple representing the
  input parameters for insertion and deletion.


```C++
template <class ADDR>
void rtacl::db::insert(entry<ADDR> const& ent);
```

Inserts a copy of **ent** into **db::rtree**.

##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)

##### Input Parameters

* **ent**: Reference to the R-tree ACL entry to be inserted.


```C++
template <class ADDR>
void rtacl::db::remove(entry<ADDR> const& ent);
```

Removes the R-tree entry matching **ent** from **db::rtree**.

##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)

##### Input Parameters

* **ent**: Reference to the R-tree ACL entry to be removed.


```C++
template <class ADDR>
rtacl::result<ADDR> rtacl::db::find(const tuple<ADDR>& key);
```

Finds R-tree ACL entries matching **key**.

##### Template Parameters

* **ADDR**: must be either **rtacl::ipv4a** (**s64**) or
  **rtacl::ipv6a** (**boost::multiprecision::int256_t**.)

##### Input Parameters

* **key**: Reference to the R-tree ACL tuple to be looked up.

##### Return Value

Vector of the matched R-tree ACL entries.


```C++
template <class ADDR>
rtacl::result<ADDR> rtacl::db::dump();
```

##### Return Value

Vector of all the R-tree ACL entries.


## Examples

The following function is a part of *unitTest.cpp*.

```C++
static void
v4rawTest ()
{
    /*
     * Sample ACL tuple and ACL entry (host byte order)
     */
    struct aclTuple {
        ipv4a sa;               // source IPv4 address
        ipv4a da;               // destination IPv4 address
        u16 sp;                 // source port number
        u16 dp;                 // destination port number
        u8 proto;               // IP protocol (TCP, UDP, etc.)
        u8 dscp;                // DSCP
    };
    struct aclEnt {
        aclTuple min;           // lower bound
        aclTuple max;           // upper bound
    };

    aclEnt ent[10];
    rtacl::db<rtacl::ipv4a> acl;
    rtacl::entry<rtacl::ipv4a> rtAclEnt;

    /*
     * Insert 10 ACL entries into R-tree
     */
    memset(ent, 0, sizeof(ent));
    size_t i;
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::tuple<rtacl::ipv4a>& min = rtAclEnt.first.min_corner();
        rtacl::tuple<rtacl::ipv4a>& max = rtAclEnt.first.max_corner();

        ent[i].min.sa = 0x0a010000 + (i << 8); // 10.1.i.0
        ent[i].max.sa = ent[i].min.sa + 10;    // 10.1.i.10
        ent[i].min.da = 0;
        ent[i].max.da = ~0;
        ent[i].min.sp = 0;
        ent[i].max.sp = ~0;
        ent[i].min.dp = 80;     // http
        ent[i].max.dp = 80;
        ent[i].min.proto = 6;   // TCP
        ent[i].max.proto = 6;
        ent[i].min.dscp = 0;
        ent[i].max.dscp = ~0;
        /*
         * sa: 10.1.i.0 - 10.1.i.10,
         * da: 0.0.0.0  - 255.255.255.255
         * sp: 0 - 65535
         * dp: 80
         * proto: TCP (6)
         * dscp: 0 - 255
         */
        acl.makeMin(ent[i].min.sa, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, min);
        acl.makeMax(ent[i].max.sa, ent[i].max.da,
                    ent[i].max.sp, ent[i].max.dp,
                    ent[i].max.proto, ent[i].max.dscp, max);
        rtAclEnt.second = reinterpret_cast<uintptr_t>(ent + i);
        acl.insert(rtAclEnt);
    }
    std::cout << (bfmt("size: %ld, i: %ld\n") % acl.size() % i).str();
    assert(i == acl.size());
    /*
     * Show all ACL entries
     */
    rtacl::result<rtacl::ipv4a> r = acl.dump();
    for (auto it : r) {
        rtacl::entry<rtacl::ipv4a> e;
        rtacl::tuple<rtacl::ipv4a>& min = e.first.min_corner();
        rtacl::tuple<rtacl::ipv4a>& max = e.first.max_corner();
        aclEnt* p = reinterpret_cast<aclEnt*>(it.second);
        acl.makeMin(p->min.sa, p->min.da,
                    p->min.sp, p->min.dp,
                    p->min.proto, p->min.dscp, min);
        acl.makeMax(p->max.sa, p->max.da,
                    p->max.sp, p->max.dp,
                    p->max.proto, p->max.dscp, max);
        std::cout << (bfmt("rtree: %s\nacl:   %s\n")
                      % rtacl::range2str(it.first)
                      % rtacl::range2str(e.first)).str();
    }
    /*
     * ACL match test (match)
     */
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::tuple<rtacl::ipv4a> key;
        acl.makeKey(ent[i].min.sa + 1, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, key);
        rtacl::result<rtacl::ipv4a> result = acl.find(key);
        if (result.size() != 1) {
            std::cout << "Error: no match. key: ";
            std::cout << (bfmt("%s\n") % rtacl::tuple2str(key)).str();
        }
        for (auto r : result) {
            assert(reinterpret_cast<uintptr_t>(ent + i) == r.second);
            std::cout << (bfmt("ent: %p, result: 0x%lx\n")
                          % (ent + i)
                          % r.second).str();
        }
    }
    /*
     * ACL match test (unmatch)
     */
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::tuple<rtacl::ipv4a> key;
        acl.makeKey(ent[i].min.sa + 11, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, key);
        rtacl::result<rtacl::ipv4a> result = acl.find(key);
        if (result.size() == 0) {
            std::cout << "no match (correct): key: ";
            std::cout << (bfmt("%s\n") % rtacl::tuple2str(key)).str();
        } else {
            std::cout << "Error: matched: key: ";
            std::cout << (bfmt("%s\n") % rtacl::tuple2str(key)).str();

            for (auto it : result) {
                std::cout << (bfmt("%s, %p\n")
                              % rtacl::range2str(it.first)
                              % it.second).str();
            }
        }
    }
    /*
     * Remove ACL entries
     */
    for (i = 0; i < elementsof(ent); ++i) {
        rtacl::entry<rtacl::ipv4a> e;
        rtacl::tuple<rtacl::ipv4a>& min = e.first.min_corner();
        rtacl::tuple<rtacl::ipv4a>& max = e.first.max_corner();
        acl.makeMin(ent[i].min.sa, ent[i].min.da,
                    ent[i].min.sp, ent[i].min.dp,
                    ent[i].min.proto, ent[i].min.dscp, min);
        acl.makeMax(ent[i].max.sa, ent[i].max.da,
                    ent[i].max.sp, ent[i].max.dp,
                    ent[i].max.proto, ent[i].max.dscp, max);
        e.second = reinterpret_cast<uintptr_t>(ent + i);
        bool rc = acl.remove(e);
        if (rc) {
            std::cout << (bfmt("size: %2ld: ") % acl.size()).str();

            rtacl::tuple<rtacl::ipv4a> key;
            acl.makeKey(ent[i].min.sa + 1, ent[i].min.da,
                        ent[i].min.sp, ent[i].min.dp,
                        ent[i].min.proto, ent[i].min.dscp, key);
            rtacl::result<rtacl::ipv4a> result = acl.find(key);
            if (result.size() == 0) {
                std::cout << (bfmt("no match (correct): key: %s\n")
                              % rtacl::tuple2str(key)).str();
            } else {
                std::cout << (bfmt("Error: matched: key: %s\n")
                              % rtacl::tuple2str(key)).str();
                for (auto it : result) {
                    std::cout << (bfmt("%s, %p\n")
                                  % rtacl::range2str(it.first)
                                  % it.second).str();
                }
            }
        } else {
            std::cout << (bfmt("Error: failed to remove acl entry: %s %ld\n")
                          % rtacl::range2str(e.first)
                          % e.second).str();
        }
    }
}
```

## Performance Details

* CPU: Intel(R) Core(TM) i3-4160T CPU @ 3.10GHz (cache size: 3072 KB)
* Memory: 4GB
* Compiler options: -O3 -DNDEBUG

### Search (matching)

830ns/search with 1 million IPv4 ACL entries.

### Search (un-matching)

780ns/search with 1 million IPv4 ACL entries.

### Insertion

3.27 us/insertion in average while inserting 1 million IPv4 ACL entries.

### Deletion

1.25 us/insertion in average while deleting 1 million IPv4 ACL entries.


```
CPU:    Intel(R) Core(TM) i3-4160T CPU @ 3.10GHz (cache size: 3072 KB)
Memory: 4GB

size: 1000000, i: 1000000
insert:  1000000 calls, 3267999.68 us, 3.27 us/call, min: 120 ns, max: 99.444 us
insert:      0ns - 1000ns:  0.56%  5609
insert:      1us -  2us:   73.88%  738780
insert:      2us -  3us:   13.71%  137123
insert:      3us -  4us:    0.07%  660
insert:      4us -  5us:    0.00%  28
insert:      5us -  6us:    0.01%  132
insert:      6us -  7us:    0.00%  12
insert:      7us -  8us:    0.00%  4
insert:      8us -  9us:    0.00%  3
insert:      9us - 10us:    0.00%  1
insert:     10us -  20us:   9.79%  97938
insert:     20us -  30us:   1.75%  17458
insert:     30us -  40us:   0.17%  1666
insert:     40us -  50us:   0.05%  488
insert:     50us -  60us:   0.01%  79
insert:     60us -  70us:   0.00%  13
insert:     70us -  80us:   0.00%  0
insert:     80us -  90us:   0.00%  5
insert:     90us - 100us:   0.00%  1
insert:    100us - 1000us:  0.00%  0
insert:           >1ms:      ---   0

Random match test:
match:  1000000 calls, 833676.50 us, 0.83 us/call, min: 226 ns, max: 19.099 us
match:      0ns -  100ns:  0.00%  0
match:    100ns -  200ns:  0.00%  0
match:    200ns -  300ns:  0.01%  114
match:    300ns -  400ns:  0.06%  564
match:    400ns -  500ns:  0.31%  3083
match:    500ns -  600ns:  2.42%  24228
match:    600ns -  700ns: 13.29%  132941
match:    700ns -  800ns: 31.47%  314720
match:    800ns -  900ns: 26.34%  263397
match:    900ns - 1000ns: 11.40%  114001
match:      1us -  2us:   14.67%  146728
match:      2us -  3us:    0.00%  16
match:      3us -  4us:    0.00%  0
match:      4us -  5us:    0.00%  0
match:      5us -  6us:    0.00%  2
match:      6us -  7us:    0.00%  30
match:      7us -  8us:    0.01%  92
match:      8us -  9us:    0.00%  6
match:      9us - 10us:    0.00%  5
match:     10us - 100us:   0.01%  73
match:    100us - 1000us:  0.00%  0
match:           >1ms:      ---   0

Random unmatch test:
unmatch:  1000000 calls, 781250.86 us, 0.78 us/call, min: 166 ns, max: 41.015 us
unmatch:      0ns -  100ns:  0.00%  0
unmatch:    100ns -  200ns:  0.00%  17
unmatch:    200ns -  300ns:  0.02%  175
unmatch:    300ns -  400ns:  0.07%  750
unmatch:    400ns -  500ns:  0.58%  5764
unmatch:    500ns -  600ns:  5.38%  53756
unmatch:    600ns -  700ns: 22.96%  229649
unmatch:    700ns -  800ns: 35.30%  352967
unmatch:    800ns -  900ns: 18.41%  184081
unmatch:    900ns - 1000ns:  7.81%  78098
unmatch:      1us -  2us:    9.45%  94543
unmatch:      2us -  3us:    0.00%  0
unmatch:      3us -  4us:    0.00%  0
unmatch:      4us -  5us:    0.00%  0
unmatch:      5us -  6us:    0.00%  3
unmatch:      6us -  7us:    0.01%  52
unmatch:      7us -  8us:    0.01%  67
unmatch:      8us -  9us:    0.00%  3
unmatch:      9us - 10us:    0.00%  5
unmatch:     10us - 100us:   0.01%  70
unmatch:    100us - 1000us:  0.00%  0
unmatch:           >1ms:      ---   0

remove:  1000000 calls, 1249830.84 us, 1.25 us/call, min: 44 ns, max: 191.102 us
remove:      0ns -  100ns:  0.00%  16
remove:    100ns -  200ns:  0.01%  70
remove:    200ns -  300ns:  0.04%  419
remove:    300ns -  400ns:  0.32%  3217
remove:    400ns -  500ns:  1.96%  19640
remove:    500ns -  600ns:  8.18%  81849
remove:    600ns -  700ns: 21.03%  210252
remove:    700ns -  800ns: 28.17%  281707
remove:    800ns -  900ns: 18.47%  184740
remove:    900ns - 1000ns:  6.81%  68074
remove:      1us -  2us:    3.23%  32350
remove:      2us -  3us:    0.23%  2292
remove:      3us -  4us:    2.04%  20387
remove:      4us -  5us:    6.67%  66653
remove:      5us -  6us:    0.96%  9646
remove:      6us -  7us:    0.25%  2462
remove:      7us -  8us:    0.65%  6536
remove:      8us -  9us:    0.64%  6401
remove:      9us - 10us:    0.14%  1409
remove:     10us - 100us:   0.19%  1879
remove:    100us - 1000us:  0.00%  1
remove:           >1ms:      ---   0
```
