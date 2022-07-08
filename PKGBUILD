pkgname=dnsd
pkgver=1
pkgrel=1
pkgdesc="simple DNS daemon"
arch=('any')
license=('MIT')
depends=('json-c')
makedepends=('make')
checkdepends=('python')
_distname="dnsd"

_distdir="${_distname}-${pkgver}"
options+=('!emptydirs')

build() {
    cd ..
    make
}

check() {
    cd ..
    #make test
}

package() {
    cd ..
    make DESTDIR="$pkgdir" install
}
