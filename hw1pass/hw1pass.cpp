#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Format.h"

#include  <iostream>

using namespace llvm;

namespace {

struct HW1Pass : public PassInfoMixin<HW1Pass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    // These variables contain the results of some analysis passes.
    // Go through the documentation to see how they can be used.
    // std::cout << "Profiling function: " << F.getName().str() << std::endl;
    llvm::BlockFrequencyAnalysis::Result &bfi = FAM.getResult<BlockFrequencyAnalysis>(F);
    llvm::BranchProbabilityAnalysis::Result &bpi = FAM.getResult<BranchProbabilityAnalysis>(F);

    // Add your code here
    // Rudra start

    uint64_t dynOpCount = 0;
    uint64_t intAluCount = 0;
    uint64_t floatAluCount = 0;
    uint64_t memCount = 0;
    uint64_t biasedBranchCount = 0;
    uint64_t unbiasedBranchCount = 0;
    uint64_t otherCount = 0;

    for (auto &BB : F) {
        // Estimate block execution frequency
        // uint64_t blockFreq = bfi.getBlockFreq(&BB).getFrequency(); // General-purpose estimate scaled to 1<<14, used when profiled data not available
        uint64_t blockFreq = bfi.getBlockProfileCount(&BB).value(); // Relative block frequency of each BB * parent function's count (accurate given data)

        for (auto &I : BB) {
            // Increment dynamic operation count
            dynOpCount += blockFreq;

            // Classify the instruction
            switch (I.getOpcode()) {
            case Instruction::Br:
            case Instruction::Switch:
            case Instruction::IndirectBr: {
                bool isBiased = false;
                if (auto *BI = dyn_cast<BranchInst>(&I)) {
                    if (BI->isConditional()) {
                      // Any branch has probability > 0.8 then it is biased
                      for (int br = 0; br < BI->getNumSuccessors(); br++){
                        auto prob = bpi.getEdgeProbability(&BB, BI->getSuccessor(br));
                        // Flag as biased if bias > 0.8
                        if (prob.getNumerator() > 0.8*prob.getDenominator()) {
                          isBiased = true;
                          break;
                        }
                      }
                    }
                    else{
                      isBiased = true; // Unconditional branch is apparently always biased?
                    }
                }
                if (isBiased) {
                    biasedBranchCount += blockFreq;
                } else {
                    unbiasedBranchCount += blockFreq;
                }
                break;
            }
            case Instruction::Add:
            case Instruction::Sub:
            case Instruction::Mul:
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::URem:
            case Instruction::Shl:
            case Instruction::LShr:
            case Instruction::AShr:
            case Instruction::And:
            case Instruction::Or:
            case Instruction::Xor:
            case Instruction::ICmp:
            case Instruction::SRem:
                intAluCount += blockFreq;
                break;
            case Instruction::FAdd:
            case Instruction::FSub:
            case Instruction::FMul:
            case Instruction::FDiv:
            case Instruction::FRem:
            case Instruction::FCmp:
                floatAluCount += blockFreq;
                break;
            case Instruction::Alloca:
            case Instruction::Load:
            case Instruction::Store:
            case Instruction::GetElementPtr:
            case Instruction::Fence:
            case Instruction::AtomicCmpXchg:
            case Instruction::AtomicRMW:
                memCount += blockFreq;
                break;
            default:
                otherCount += blockFreq;
                break;
            }
        }
    }

    auto frac = [](uint64_t part, uint64_t total) {
        return total == 0 ? 0.0 : (static_cast<double>(part) / static_cast<double>(total));
    };

    double ialuFrac = frac(intAluCount, dynOpCount);
    double faluFrac = frac(floatAluCount, dynOpCount);
    double memFrac = frac(memCount, dynOpCount);
    double biasedBranchFrac = frac(biasedBranchCount, dynOpCount);
    double unbiasedBranchFrac = frac(unbiasedBranchCount, dynOpCount);
    double otherFrac = frac(otherCount, dynOpCount);

    // Print results
    // std::cout.setf(std::ios::fixed,std::ios::floatfield);
    // std::cout.precision(3);
    // std::cout << F.getName().str() << ", " << dynOpCount << ", "
    //           << ialuFrac << ", " << faluFrac << ", " << memFrac << ", "
    //           << biasedBranchFrac << ", " << unbiasedBranchFrac << ", "
    //           << otherFrac << std::endl;
    errs() << F.getName().str() << ", " << dynOpCount << ", "
              << format("%.3f", ialuFrac) << ", " << format("%.3f", faluFrac) << ", " << format("%.3f", memFrac) << ", "
              << format("%.3f", biasedBranchFrac) << ", " << format("%.3f", unbiasedBranchFrac) << ", "
              << format("%.3f", otherFrac) << "\n";
    // Rudra end

    return PreservedAnalyses::all();
  }
};
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "HW1Pass", "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
        ArrayRef<PassBuilder::PipelineElement>) {
          if(Name == "hw1"){
            FPM.addPass(HW1Pass());
            return true;
          }
          return false;
        }
      );
    }
  };
}