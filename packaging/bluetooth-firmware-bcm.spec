Name:       bluetooth-firmware-bcm
Summary:    firmware and tools for bluetooth
Version:    0.2.29
Release:    1
Group:      Hardware Support/Handset
License:    Apache
# NOTE: Source name does not match package name.  This should be
# resolved in the future, by I don't have that power. - Ryan Ware
Source0:    bluetooth-firmware-bcm-%{version}.tar.gz

BuildRequires:  cmake

%description
firmware and tools for bluetooth

%package c210
Summary:    c210 firmware and tools for bluetooth
Group:      TO_BE/FILLED

%description c210
c210 firmware and tools for bluetooth

%package e4412
Summary:    e4412 firmware and tools for bluetooth
Group:      TO_BE/FILLED

%description e4412
e4412 firmware and tools for bluetooth

%prep
%setup -q -n bluetooth-firmware-bcm-%{version}

%build
export CFLAGS+=" -fpie -fvisibility=hidden"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"

cmake ./ -DCMAKE_INSTALL_PREFIX=%{_prefix} -DPLUGIN_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
cat %{_builddir}/%{name}-%{version}/LICENSE.Broadcom >> %{buildroot}/usr/share/license/%{name}

%post c210
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%post e4412
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%files c210
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4330B1_002.001.003.0221.0265.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}

%files e4412
%manifest bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.0079.0081.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
/usr/share/license/%{name}
