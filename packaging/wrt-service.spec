Name:       wrt-service
Summary:    Service Model for Web Runtime
Version:    0.1.2
Release:    1
Group:      Development/Libraries
License:    Apache-2.0
URL:        N/A
Source0:    %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(nodejs)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(security-client)
BuildRequires: pkgconfig(libprivilege-control)
Requires: nodejs

%description
Providing service model for tizen web runtime.

%package -n wrt-common-native
Summary:    Common libraries for bridge between JS and native
Group:      Development/Libraries

%description -n wrt-common-native
Common libraries for bridge between JS and native

%package -n wrt-common-native-devel
Summary:    Common libraries for bridge between JS and native (Development)
Group:      Development/Libraries
Requires:   wrt-common-native

%description -n wrt-common-native-devel
Development package for wrt-common-native

%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`

export LDFLAGS="$LDFLAGS -Wl,--rpath=/usr/lib -Wl,--hash-style=both -Wl,--as-needed"

mkdir -p cmake_build_tmp
cd cmake_build_tmp

cmake .. \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_BUILD_TYPE=%{?build_type:%build_type} \
        -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}

cd cmake_build_tmp
%make_install

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest wrt-service.manifest
%{_datadir}/license/%{name}
%attr(755,root,root) %{_bindir}/wrt-service
%{_libdir}/wrt-service/*.node


%files -n wrt-common-native
%manifest wrt-service.manifest
%{_libdir}/lib*.so.*

%files -n wrt-common-native-devel
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/*.pc
%{_includedir}/wrt-common/*.h
