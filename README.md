# rtacl::db - An R-tree based ACL

## Introduction

**rtacl::db** is an ACL (Access Control List) C++ class using
R-tree. **rtacl::db** supports both IPv4 and IPv6 address
ranges as well as TCP/UDP port range, IP protocol range, and
DSCP range although IP protocol and DSCP may not need range
matching.

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
