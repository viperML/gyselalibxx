#include <array>

#include <sll/gauss_legendre_integration.hpp>

/// for i=1..n, w_i, x_i
///
/// Coefficients taken from
/// http://www.holoborodko.com/pavel/numerical-methods/numerical-integration
/// The `arb` library is also able to produce weights and nodes at
/// arbitrary precision (see arb_hypgeom_legendre_p_ui_root) by
/// implementing algorithm from F. Johansson & M. Mezzarobba (2018)
std::array<long double, GaussLegendreCoefficients::nb_coefficients>
        GaussLegendreCoefficients::weight
        = {2.0000000000000000000000000l,
           // order 2
           1.0000000000000000000000000l,
           1.0000000000000000000000000l,
           // order 3
           0.8888888888888888888888889l,
           0.5555555555555555555555556l,
           0.5555555555555555555555556l,
           // order 4
           0.6521451548625461426269361l,
           0.6521451548625461426269361l,
           0.3478548451374538573730639l,
           0.3478548451374538573730639l,
           // order 5
           0.5688888888888888888888889l,
           0.4786286704993664680412915l,
           0.4786286704993664680412915l,
           0.2369268850561890875142640l,
           0.2369268850561890875142640l,
           // order 6
           0.4679139345726910473898703l,
           0.4679139345726910473898703l,
           0.3607615730481386075698335l,
           0.3607615730481386075698335l,
           0.1713244923791703450402961l,
           0.1713244923791703450402961l,
           // order 7
           0.4179591836734693877551020l,
           0.3818300505051189449503698l,
           0.3818300505051189449503698l,
           0.2797053914892766679014678l,
           0.2797053914892766679014678l,
           0.1294849661688696932706114l,
           0.1294849661688696932706114l,
           // order 8
           0.3626837833783619829651504l,
           0.3626837833783619829651504l,
           0.3137066458778872873379622l,
           0.3137066458778872873379622l,
           0.2223810344533744705443560l,
           0.2223810344533744705443560l,
           0.1012285362903762591525314l,
           0.1012285362903762591525314l,
           // order 9
           0.3302393550012597631645251l,
           0.3123470770400028400686304l,
           0.3123470770400028400686304l,
           0.2606106964029354623187429l,
           0.2606106964029354623187429l,
           0.1806481606948574040584720l,
           0.1806481606948574040584720l,
           0.0812743883615744119718922l,
           0.0812743883615744119718922l,
           // order 10
           0.2955242247147528701738930l,
           0.2955242247147528701738930l,
           0.2692667193099963550912269l,
           0.2692667193099963550912269l,
           0.2190863625159820439955349l,
           0.2190863625159820439955349l,
           0.1494513491505805931457763l,
           0.1494513491505805931457763l,
           0.0666713443086881375935688l,
           0.0666713443086881375935688l};

std::array<long double, GaussLegendreCoefficients::nb_coefficients> GaussLegendreCoefficients::pos
        = {+0.0000000000000000000000000l,
           // order 2
           -0.5773502691896257645091488l,
           +0.5773502691896257645091488l,
           // order 3
           +0.0000000000000000000000000l,
           -0.7745966692414833770358531l,
           +0.7745966692414833770358531l,
           // order 4
           -0.3399810435848562648026658l,
           +0.3399810435848562648026658l,
           -0.8611363115940525752239465l,
           +0.8611363115940525752239465l,
           // order 5
           +0.0000000000000000000000000l,
           -0.5384693101056830910363144l,
           +0.5384693101056830910363144l,
           -0.9061798459386639927976269l,
           +0.9061798459386639927976269l,
           // order 6
           -0.2386191860831969086305017l,
           +0.2386191860831969086305017l,
           -0.6612093864662645136613996l,
           +0.6612093864662645136613996l,
           -0.9324695142031520278123016l,
           +0.9324695142031520278123016l,
           // order 7
           +0.0000000000000000000000000l,
           -0.4058451513773971669066064l,
           +0.4058451513773971669066064l,
           -0.7415311855993944398638648l,
           +0.7415311855993944398638648l,
           -0.9491079123427585245261897l,
           +0.9491079123427585245261897l,
           // order 8
           -0.1834346424956498049394761l,
           +0.1834346424956498049394761l,
           -0.5255324099163289858177390l,
           +0.5255324099163289858177390l,
           -0.7966664774136267395915539l,
           +0.7966664774136267395915539l,
           -0.9602898564975362316835609l,
           +0.9602898564975362316835609l,
           // order 9
           +0.0000000000000000000000000l,
           -0.3242534234038089290385380l,
           +0.3242534234038089290385380l,
           -0.6133714327005903973087020l,
           +0.6133714327005903973087020l,
           -0.8360311073266357942994298l,
           +0.8360311073266357942994298l,
           -0.9681602395076260898355762l,
           +0.9681602395076260898355762l,
           // order 10
           -0.1488743389816312108848260l,
           +0.1488743389816312108848260l,
           -0.4333953941292471907992659l,
           +0.4333953941292471907992659l,
           -0.6794095682990244062343274l,
           +0.6794095682990244062343274l,
           -0.8650633666889845107320967l,
           +0.8650633666889845107320967l,
           -0.9739065285171717200779640l,
           +0.9739065285171717200779640l};
