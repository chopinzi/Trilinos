#! /usr/bin/env python

# @HEADER
# ************************************************************************
#
#                PyTrilinos: Python Interface to Trilinos
#                   Copyright (2005) Sandia Corporation
#
# Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
# license for use of this work by or on behalf of the U.S. Government.
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
# Questions? Contact Michael A. Heroux (maherou@sandia.gov)
#
# ************************************************************************
# @HEADER

# Imports.  Users importing an installed version of PyTrilinos should use the
# "from PyTrilinos import ..." syntax.  Here, the setpath module adds the build
# directory, including "PyTrilinos", to the front of the search path.  We thus
# use "import ..." for Trilinos modules.  This prevents us from accidentally
# picking up a system-installed version and ensures that we are testing the
# build module.
from   numpy    import *
from   optparse import *
import sys
import unittest

parser = OptionParser()
parser.add_option("-b", "--use-boost", action="store_true", dest="boost",
                  default=False,
                  help="test the experimental boost-generated PyTrilinos package")
parser.add_option("-t", "--testharness", action="store_true",
                  dest="testharness", default=False,
                  help="test local build modules; prevent loading system-installed modules")
parser.add_option("-v", "--verbosity", type="int", dest="verbosity", default=2,
                  help="set the verbosity level [default 2]")
options,args = parser.parse_args()
if options.testharness:
    import setpath
    if options.boost: setpath.setpath("src-boost")
    else:             setpath.setpath()
    import Epetra
else:
    try:
        import setpath
        if options.boost: setpath.setpath("src-boost")
        else:             setpath.setpath()
        import Epetra
    except ImportError:
        from PyTrilinos import Epetra
        print >>sys.stderr, "Using system-installed Epetra"

##########################################################################

class EpetraLocalMapTestCase(unittest.TestCase):
    "TestCase class for LocalMap objects"

    def setUp(self):
        self.comm             = Epetra.PyComm()
        self.numProc          = self.comm.NumProc()
        self.myPID            = self.comm.MyPID()
        self.numElem          = 4
        self.elSize           = 1
        self.indexBase        = 0
        self.map              = Epetra.LocalMap(self.numElem,
                                                self.indexBase,
                                                self.comm)
        self.comm.Barrier()

    def tearDown(self):
        self.comm.Barrier()

    def testConstructor1(self):
        "Test Epetra.LocalMap uniform, linear constructor"
        self.assertEqual(self.map.NumGlobalElements(), self.numElem  )
        self.assertEqual(self.map.ElementSize(),       self.elSize   )
        self.assertEqual(self.map.IndexBase(),         self.indexBase)

    def testConstructor2(self):
        "Test Epetra.LocalMap copy constructor"
        map = Epetra.LocalMap(self.map)
        self.assertEqual(self.map.SameAs(map),True)

    def testRemoteIDList(self):
        "Test Epetra.LocalMap RemoteIDList method for constant element size"
        gidList  = range(self.map.NumGlobalElements())
        sizeList = ones(  self.map.NumGlobalElements())
        pidList  = sizeList * self.myPID
        lidList  = gidList
        result = self.map.RemoteIDList(gidList)
        for id in range(len(gidList)):
            self.assertEqual(result[0][id], pidList[id] )
            self.assertEqual(result[1][id], lidList[id] )
            self.assertEqual(result[2][id], sizeList[id])

    def testLID(self):
        "Test Epetra.LocalMap LID method"
        for gid in range(self.map.NumGlobalElements()):
            self.assertEqual(self.map.LID(gid),gid)

    def testGID(self):
        "Test Epetra.LocalMap GID method"
        for lid in range(self.map.NumMyElements()):
            self.assertEqual(self.map.GID(lid),lid)

    def testFindLocalElementID(self):
        "Test Epetra.LocalMap FindLocalElementID method"
        pointID = 0
        for lid in range(self.map.NumMyElements()):
            result = self.map.FindLocalElementID(pointID)
            self.assertEqual(result[0], lid)
            self.assertEqual(result[1], 0  )
            pointID += 1

    def testMyGID(self):
        "Test Epetra.LocalMap MyGID method"
        for gid in range(self.map.NumGlobalElements()):
            self.assertEqual(self.map.MyGID(gid), True)

    def testMyLID(self):
        "Test Epetra.LocalMap MyLID method"
        for lid in range(self.map.NumGlobalElements()):
            self.assertEqual(self.map.MyLID(lid), True)

    def testMinAllGID(self):
        "Test Epetra.LocalMap MinAllGID method"
        self.assertEqual(self.map.MinAllGID(), self.indexBase)

    def testMaxAllGID(self):
        "Test Epetra.LocalMap MaxAllGID method"
        self.assertEqual(self.map.MaxAllGID(), self.numElem-1)

    def testMinMyGID(self):
        "Test Epetra.LocalMap MinMyGID method"
        self.assertEqual(self.map.MinMyGID(), self.indexBase)

    def testMaxMyGID(self):
        "Test Epetra.LocalMap MaxMyGID method"
        self.assertEqual(self.map.MaxMyGID(), self.numElem-1)

    def testMinLID(self):
        "Test Epetra.LocalMap MinLID method"
        self.assertEqual(self.map.MinLID(), self.indexBase)

    def testMaxLID(self):
        "Test Epetra.LocalMap MaxLID method"
        self.assertEqual(self.map.MaxLID(), self.numElem-1)

    def testNumGlobalElements(self):
        "Test Epetra.LocalMap NumGlobalElements method"
        self.assertEqual(self.map.NumGlobalElements(), self.numElem)

    def testNumMyElements(self):
        "Test Epetra.LocalMap NumMyElements method"
        self.assertEqual(self.map.NumMyElements(), self.numElem)

    def testMyGlobalElements(self):
        "Test Epetra.LocalMap MyGlobalElements method"
        elements = arange(self.map.NumGlobalElements())
        result   = self.map.MyGlobalElements()
        for id in range(len(result)):
            self.assertEqual(result[id],elements[id])

    def testElementSize1(self):
        "Test Epetra.LocalMap ElementSize method"
        self.assertEqual(self.map.ElementSize(), self.elSize)

    def testElementSize2(self):
        "Test Epetra.LocalMap ElementSize method for specified LID"
        for lid in range(self.map.NumMyElements()):
            self.assertEqual(self.map.ElementSize(lid), self.elSize)

    def testFirstPointInElement(self):
        "Test Epetra.LocalMap FirstPointInElement method"
        for lid in range(self.map.NumMyElements()):
            self.assertEqual(self.map.FirstPointInElement(lid),lid)

    def testIndexBase(self):
        "Test Epetra.LocalMap IndexBase method"
        self.assertEqual(self.map.IndexBase(), self.indexBase)

    def testNumGlobalPoints(self):
        "Test Epetra.LocalMap NumGlobalPoints method"
        self.assertEqual(self.map.NumGlobalPoints(), self.numElem)

    def testNumMyPoints(self):
        "Test Epetra.LocalMap NumMyPoints method"
        self.assertEqual(self.map.NumMyPoints(), self.numElem)

    def testMinMyElementSize(self):
        "Test Epetra.LocalMap MinMyElementSize method"
        self.assertEqual(self.map.MinMyElementSize(), self.elSize)

    def testMaxMyElementSize(self):
        "Test Epetra.LocalMap MaxMyElementSize method"
        self.assertEqual(self.map.MaxMyElementSize(), self.elSize)

    def testMinElementSize(self):
        "Test Epetra.LocalMap MinElementSize method"
        self.assertEqual(self.map.MinElementSize(), self.elSize)

    def testMaxElementSize(self):
        "Test Epetra.LocalMap MaxElementSize method"
        self.assertEqual(self.map.MaxElementSize(), self.elSize)

    def testConstantElementSize(self):
        "Test Epetra.LocalMap ConstantElementSize method"
        self.assertEqual(self.map.ConstantElementSize(), True )

    def testSameAs(self):
        "Test Epetra.LocalMap SameAs method"
        self.assertEqual(self.map.SameAs(self.map), True)

    def testPointSameAs(self):
        "Test Epetra.LocalMap PointSameAs method"
        self.assertEqual(self.map.PointSameAs(self.map), True)

    def testLinearMap(self):
        "Test Epetra.LocalMap LinearMap method"
        self.assertEqual(self.map.LinearMap(), True )

    def testDistributedGlobal(self):
        "Test Epetra.LocalMap DistributedGlobal method"
        self.assertEqual(self.map.DistributedGlobal(), False)

    def testFirstPointInElementList(self):
        "Test Epetra.LocalMap FirstPointInElementList method"
        firstPoints = [lid for lid in range(self.numElem)]
        result      = self.map.FirstPointInElementList()
        for lid in range(self.numElem):
            self.assertEqual(result[lid], firstPoints[lid])

    def testElementSizeList(self):
        "Test Epetra.LocalMap ElementSizeList method"
        size   = [self.elSize for lid in range(self.numElem)]
        result = self.map.ElementSizeList()
        for lid in range(self.numElem):
            self.assertEqual(result[lid], size[lid])

    def testPointToElementList(self):
        "Test Epetra.LocalMap PointToElementList method"
        elementList = range(self.map.NumMyElements())
        result      = self.map.PointToElementList()
        for pointID in range(len(elementList)):
            self.assertEqual(result[pointID], elementList[pointID])

    def testStr(self):
        "Test Epetra.LocalMap __str__ method"
        lines   = 7 + self.numElem
        if self.myPID == 0: lines += 7
        s = str(self.map)
        s = s.splitlines()
        self.assertEquals(len(s), lines)

    def testPrint(self):
        "Test Epetra.LocalMap Print method"
        myPID = self.myPID
        filename = "testLocalMap%d.dat" % myPID
        f = open(filename, "w")
        self.map.Print(f)
        f.close()
        s = open(filename, "r").readlines()
        lines = 7 + self.numElem
        if myPID == 0: lines += 7
        self.assertEquals(len(s), lines)

    def testComm(self):
        "Test Epetra.LocalMap Comm method"
        comm = self.map.Comm()
        self.assertEqual(comm.NumProc(),self.comm.NumProc())
        self.assertEqual(comm.MyPID()  ,self.comm.MyPID()  )

##########################################################################

if __name__ == "__main__":

    # Create the test suite object
    suite = unittest.TestSuite()

    # Add the test cases to the test suite
    suite.addTest(unittest.makeSuite(EpetraLocalMapTestCase))

    # Create a communicator
    comm    = Epetra.PyComm()
    iAmRoot = comm.MyPID() == 0

    # Run the test suite
    if iAmRoot: print >>sys.stderr, \
       "\n***********************\nTesting Epetra.LocalMap\n***********************\n"
    verbosity = options.verbosity * int(iAmRoot)
    result = unittest.TextTestRunner(verbosity=verbosity).run(suite)

    # Exit with a code that indicates the total number of errors and failures
    errsPlusFails = comm.SumAll(len(result.errors) + len(result.failures))
    if errsPlusFails == 0 and iAmRoot: print "End Result: TEST PASSED"
    sys.exit(errsPlusFails)
