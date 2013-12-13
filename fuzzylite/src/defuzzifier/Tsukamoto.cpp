
#include "fl/defuzzifier/Tsukamoto.h"

#include "fl/term/Thresholded.h"

#include "fl/term/Ramp.h"
#include "fl/term/Sigmoid.h"
#include "fl/term/SShape.h"
#include "fl/term/ZShape.h"

namespace fl {

    scalar Tsukamoto::tsukamoto(const Thresholded* term, scalar minimum, scalar maximum) {
        const Term* monotonic = term->getTerm();
        const Ramp* ramp = NULL;
        const Sigmoid* sigmoid = NULL;
        const SShape* sshape = NULL;
        const ZShape* zshape = NULL;
        scalar w = term->getThreshold();
        scalar z = fl::nan; //result;
        if ((ramp = dynamic_cast<const Ramp*> (monotonic))) {
            z = Op::scale(w, 0, 1, ramp->getStart(), ramp->getEnd());

        } else if ((sigmoid = dynamic_cast<const Sigmoid*> (monotonic))) {
            if (Op::isEq(w, 1.0)) {
                if (Op::isGE(sigmoid->getSlope(), 0.0)) {
                    z = maximum;
                } else {
                    z = minimum;
                }

            } else if (Op::isEq(w, 0.0)) {
                if (Op::isGE(sigmoid->getSlope(), 0.0)) {
                    z = minimum;
                } else {
                    z = maximum;
                }
            } else {
                scalar a = sigmoid->getSlope();
                scalar b = sigmoid->getInflection();
                z = b + (std::log(1.0 / w - 1.0) / -a);
            }

        } else if ((sshape = dynamic_cast<const SShape*> (monotonic))) {
            scalar difference = sshape->getEnd() - sshape->getStart();
            scalar a = sshape->getStart() + std::sqrt(w * difference * difference / 2.0);
            scalar b = sshape->getEnd() + std::sqrt(difference * difference * (w - 1.0) / -2.0);
            if (Op::isLE(std::fabs(w - monotonic->membership(a)),
                    std::fabs(w - monotonic->membership(b)))) {
                z = a;
            } else {
                z = b;
            }

        } else if ((zshape = dynamic_cast<const ZShape*> (monotonic))) {
            scalar difference = zshape->getEnd() - zshape->getStart();
            scalar a = zshape->getStart() + std::sqrt(difference * difference * (w - 1) / -2.0);
            scalar b = zshape->getEnd() + std::sqrt(difference * difference * w / 2.0);
            if (Op::isLE(std::fabs(w - monotonic->membership(a)),
                    std::fabs(w - monotonic->membership(b)))) {
                z = a;
            } else {
                z = b;
            }

        } else {
            //This will work as an Inverse Tsukamoto.
            z = monotonic->membership(term->getThreshold());
        }

        scalar fz = monotonic->membership(z);
        if (not Op::isEq(w, fz, fuzzylite::macheps() * 1e-2)) {
            FL_LOG("[tsukamoto error] expected w=f(z) in " << monotonic->className() <<
                    " term <" << monotonic->getName() << ">, but "
                    "w=" << term->getThreshold() << " "
                    "f(z)=" << fz << " "
                    "z=" << Op::str(z) << " (inaccurate computation of z)");
        }
        return z;
    }

}
