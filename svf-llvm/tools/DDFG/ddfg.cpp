#include "SVF-LLVM/LLVMUtil.h"
#include "Graphs/SVFG.h"
#include "Util/SVFUtil.h"
#include "WPA/Andersen.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "Util/CommandLine.h"
#include "Util/Options.h"

using namespace std;
using namespace SVF;

int main(int argc, char **argv)
{
    std::vector<std::string> moduleNameVec;
    moduleNameVec = OptionBase::parseOptions(
        argc, argv, "Distributed Data-Flow Analysis", "[options] <input-bitcode...>");

    SVFModule   *svfModule = LLVMModuleSet::buildSVFModule(moduleNameVec);
    SVFIRBuilder builder(svfModule);
    SVFIR       *pag = builder.build();

    Andersen *ander = AndersenWaveDiff::createAndersenWaveDiff(pag);

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
    SVFG       *svfg = svfBuilder.buildFullSVFG(ander);
    svfg->dump("svfg");

    // clean up memory
    delete vfg;
    AndersenWaveDiff::releaseAndersenWaveDiff();
    SVFIR::releaseSVFIR();

    LLVMModuleSet::getLLVMModuleSet()->dumpModulesToFile(".svf.bc");
    SVF::LLVMModuleSet::releaseLLVMModuleSet();

    llvm::llvm_shutdown();

    return 0;
}
