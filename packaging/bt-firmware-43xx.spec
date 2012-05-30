Name:       bt-firmware-43xx
Summary:    firmware and tools for bluetooth
Version:    0.1.2
Release:    1
Group:      TO_BE_FILLED
License:    TO_BE_FILLED
Source0:    bluetooth-firmware-bcm-%{version}.tar.gz
Source1001: packaging/bt-firmware-43xx.manifest 

BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  cmake

%description
 firmware and tools for bluetooth


%prep
%setup -q

%build
cp %{SOURCE1001} .
cmake ./ -DCMAKE_INSTALL_PREFIX=%{_prefix} -DPLUGIN_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install


%files
%manifest bt-firmware-43xx.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4330B1_002.001.003.0221.0265.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
