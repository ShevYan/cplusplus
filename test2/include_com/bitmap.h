#ifndef __BITMAP_H__
#define __BITMAP_H__
//
//  BitMap routines.  The following structure, routines, and macros are
//  for manipulating bitmaps.  The user is responsible for allocating a bitmap
//  structure (which is really a header) and a buffer (which must be longword
//  aligned and multiple longwords in size).
//
#include "BaseType.h"



#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;                     // Number of bits in bit map
    PULONG Buffer;                          // Pointer to the bit map itself
} RTL_BITMAP;
typedef RTL_BITMAP *PRTL_BITMAP;


#if defined(_M_AMD64)
VOID
RtlFillMemoryUlong (
					OUT PVOID Destination,
					IN SIZE_T Length,
					IN ULONG Pattern
					)
					
{
	
    PULONG Address = (PULONG)Destination;
	
    //
    // If the number of DWORDs is not zero, then fill the specified buffer
    // with the specified pattern.
    //
	
    if ((Length /= 4) != 0) {
		
        //
        // If the destination is not quadword aligned (ignoring low bits),
        // then align the destination by storing one DWORD.
        //
		
        if (((ULONG64)Address & 4) != 0) {
            *Address = Pattern;
            if ((Length -= 1) == 0) {
                return;
            }
			
            Address += 1;
        }
		
        //
        // If the number of QWORDs is not zero, then fill the destination
        // buffer a QWORD at a time.
        //
		
		__stosq((PULONG64)(Address),
			Pattern | ((ULONG64)Pattern << 32),
			Length / 2);
		
        if ((Length & 1) != 0) {
            Address[Length - 1] = Pattern;
        }
    }
	
    return;
}

#define RtlFillMemoryUlonglong(Destination, Length, Pattern)                \
__stosq((PULONG64)(Destination), Pattern, (Length) / 8)


#else


VOID
RtlFillMemoryUlong (
					OUT PVOID Destination,
					IN SIZE_T Length,
					IN ULONG Pattern
					);



VOID
RtlFillMemoryUlonglong (
						OUT PVOID Destination,
						IN SIZE_T Length,
						IN ULONGLONG Pattern
						);


#endif // defined(_M_AMD64)




//
//  The following routine initializes a new bitmap.  It does not alter the
//  data currently in the bitmap.  This routine must be called before
//  any other bitmap routine/macro.
//


VOID
RtlInitializeBitMap (
    OUT PRTL_BITMAP BitMapHeader,
    IN  PULONG BitMapBuffer,
    IN ULONG SizeOfBitMap
    );


//
//  The following three routines clear, set, and test the state of a
//  single bit in a bitmap.
//

VOID
RtlClearBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    );



VOID
RtlSetBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    );



BOOLEAN
RtlTestBit (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG BitNumber
    );


//
//  The following two routines either clear or set all of the bits
//  in a bitmap.
//


VOID
RtlClearAllBits (
    IN PRTL_BITMAP BitMapHeader
    );


VOID
RtlSetAllBits (
    IN PRTL_BITMAP BitMapHeader
    );


//
//  The following two routines locate a contiguous region of either
//  clear or set bits within the bitmap.  The region will be at least
//  as large as the number specified, and the search of the bitmap will
//  begin at the specified hint index (which is a bit index within the
//  bitmap, zero based).  The return value is the bit index of the located
//  region (zero based) or -1 (i.e., 0xffffffff) if such a region cannot
//  be located
//


ULONG
RtlFindClearBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    );


ULONG
RtlFindSetBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    );

//
//  The following two routines locate a contiguous region of either
//  clear or set bits within the bitmap and either set or clear the bits
//  within the located region.  The region will be as large as the number
//  specified, and the search for the region will begin at the specified
//  hint index (which is a bit index within the bitmap, zero based).  The
//  return value is the bit index of the located region (zero based) or
//  -1 (i.e., 0xffffffff) if such a region cannot be located.  If a region
//  cannot be located then the setting/clearing of the bitmap is not performed.
//


ULONG
RtlFindClearBitsAndSet (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    );

ULONG
RtlFindSetBitsAndClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG NumberToFind,
    IN ULONG HintIndex
    );

//
//  The following two routines clear or set bits within a specified region
//  of the bitmap.  The starting index is zero based.
//

VOID
RtlClearBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG NumberToClear
    );


VOID
RtlSetBits (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG NumberToSet
    );

//
//  The following routine locates a set of contiguous regions of clear
//  bits within the bitmap.  The caller specifies whether to return the
//  longest runs or just the first found lcoated.  The following structure is
//  used to denote a contiguous run of bits.  The two routines return an array
//  of this structure, one for each run located.
//

typedef struct _RTL_BITMAP_RUN {

    ULONG StartingIndex;
    ULONG NumberOfBits;

} RTL_BITMAP_RUN;
typedef RTL_BITMAP_RUN *PRTL_BITMAP_RUN;

ULONG
RtlFindClearRuns (
    IN PRTL_BITMAP BitMapHeader,
    OUT PRTL_BITMAP_RUN RunArray,
    IN ULONG SizeOfRunArray,
    IN BOOLEAN LocateLongestRuns
    );

//
//  The following routine locates the longest contiguous region of
//  clear bits within the bitmap.  The returned starting index value
//  denotes the first contiguous region located satisfying our requirements
//  The return value is the length (in bits) of the longest region found.
//


ULONG
RtlFindLongestRunClear (
    IN PRTL_BITMAP BitMapHeader,
    OUT PULONG StartingIndex
    );

//
//  The following routine locates the first contiguous region of
//  clear bits within the bitmap.  The returned starting index value
//  denotes the first contiguous region located satisfying our requirements
//  The return value is the length (in bits) of the region found.
//


ULONG
RtlFindFirstRunClear (
    IN PRTL_BITMAP BitMapHeader,
    OUT PULONG StartingIndex
    );


//
//  The following macro returns the value of the bit stored within the
//  bitmap at the specified location.  If the bit is set a value of 1 is
//  returned otherwise a value of 0 is returned.
//
//      ULONG
//      RtlCheckBit (
//          PRTL_BITMAP BitMapHeader,
//          ULONG BitPosition
//          );
//
//
//  To implement CheckBit the macro retrieves the longword containing the
//  bit in question, shifts the longword to get the bit in question into the
//  low order bit position and masks out all other bits.
//

#if defined(_M_AMD64) && !defined(MIDL_PASS)

__checkReturn
FORCEINLINE
BOOLEAN
RtlCheckBit (
    IN PRTL_BITMAP BitMapHeader,
    IN_range(<, BitMapHeader->SizeOfBitMap) ULONG BitPosition
    )

{
    return BitTest64((LONG64 const *)BitMapHeader->Buffer, (LONG64)BitPosition);
}

#else

#define RtlCheckBit(BMH,BP) (((((PLONG)(BMH)->Buffer)[(BP) / 32]) >> ((BP) % 32)) & 0x1)

#endif

//
//  The following two procedures return to the caller the total number of
//  clear or set bits within the specified bitmap.
//


ULONG
RtlNumberOfClearBits (
    IN PRTL_BITMAP BitMapHeader
    );



ULONG
RtlNumberOfSetBits (
    IN PRTL_BITMAP BitMapHeader
    );


//
//  The following two procedures return to the caller a boolean value
//  indicating if the specified range of bits are all clear or set.
//


BOOLEAN
RtlAreBitsClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG Length
    );

BOOLEAN
RtlAreBitsSet (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG StartingIndex,
    IN ULONG Length
    );


ULONG
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    OUT PULONG StartingRunIndex
    );

ULONG
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    OUT PULONG StartingRunIndex
    );

//
//  The following two procedures return to the caller a value indicating
//  the position within a ULONGLONG of the most or least significant non-zero
//  bit.  A value of zero results in a return value of -1.
//


CCHAR
RtlFindLeastSignificantBit (
    IN ULONGLONG Set
    );

CCHAR
RtlFindMostSignificantBit (
    IN ULONGLONG Set
    );

//
// The following procedure finds the number of set bits within a ULONG_PTR
// value.
//

ULONG
RtlNumberOfSetBitsUlongPtr (
    IN ULONG_PTR Target
    );


#endif