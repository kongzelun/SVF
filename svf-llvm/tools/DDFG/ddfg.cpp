#include "Graphs/SVFG.h"
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "Util/CommandLine.h"
#include "Util/Options.h"
#include "Util/SVFUtil.h"
#include "WPA/Andersen.h"

using namespace std;
using namespace SVF;

void traverseOnICFG(ICFG *icfg, const SVFInstruction *svfInst)
{
    ICFGNode                      *iNode = icfg->getICFGNode(svfInst);
    FIFOWorkList<const ICFGNode *> worklist;
    Set<const ICFGNode *>          visited;
    worklist.push(iNode);

    /// Traverse along VFG
    while (!worklist.empty())
    {
        const ICFGNode *vNode = worklist.pop();
        for (ICFGNode::const_iterator it = vNode->OutEdgeBegin(), eit =
                                                                      vNode->OutEdgeEnd();
             it != eit; ++it)
        {
            ICFGEdge *edge = *it;
            ICFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
            }
        }
    }
}

void traverseOnVFG(const SVFG *vfg, SVFValue *val)
{
    SVFIR *pag = SVFIR::getPAG();

    PAGNode                      *pNode = pag->getGNode(pag->getValueNode(val));
    const VFGNode                *vNode = vfg->getDefSVFGNode(pNode);
    FIFOWorkList<const VFGNode *> worklist;
    Set<const VFGNode *>          visited;
    worklist.push(vNode);

    /// Traverse along VFG
    while (!worklist.empty())
    {
        const VFGNode *vNode = worklist.pop();
        for (VFGNode::const_iterator it = vNode->OutEdgeBegin(), eit = vNode->OutEdgeEnd();
             it != eit; ++it)
        {
            VFGEdge *edge = *it;
            VFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
            }
        }
    }

    /// Collect all LLVM Values
    for (Set<const VFGNode *>::const_iterator it = visited.begin(), eit = visited.end(); it != eit; ++it)
    {
        // const VFGNode* node = *it;
        /// can only query VFGNode involving top-level pointers (starting with % or @ in LLVM IR)
        /// PAGNode* pNode = vfg->getLHSTopLevPtr(node);
        /// SVFValue* val = pNode->getValue();
    }
}

int main(int argc, char **argv)
{
    std::vector<std::string> moduleNameVec;
    moduleNameVec =
        OptionBase::parseOptions(argc, argv,
                                 "Distributed Data-Flow Analysis",
                                 "[options] <input-bitcode...>");

    SVFModule   *svfModule = LLVMModuleSet::buildSVFModule(moduleNameVec);
    SVFIRBuilder builder(svfModule);
    SVFIR       *pag = builder.build();
    pag->dump("pag");

    Andersen *ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    ander->disablePrintStat();

    /// Call Graph
    PTACallGraph *callgraph = ander->getPTACallGraph();
    callgraph->dump("callgraph");

    /// ICFG
    ICFG *icfg = pag->getICFG();
    icfg->dump("icfg");

    /// Value-Flow Graph (VFG)
    VFG *vfg = new VFG(callgraph);
    vfg->dump("vfg");

    /// Sparse value-flow graph (SVFG)
    SVFGBuilder svfBuilder(true);
    SVFG       *full_svfg = svfBuilder.buildFullSVFG(ander);
    full_svfg->dump("full_svfg");

    // clean up memory
    delete vfg;
    AndersenWaveDiff::releaseAndersenWaveDiff();
    SVFIR::releaseSVFIR();

    LLVMModuleSet::getLLVMModuleSet()->dumpModulesToFile(".svf.bc");
    SVF::LLVMModuleSet::releaseLLVMModuleSet();

    llvm::llvm_shutdown();

    return 0;
}
