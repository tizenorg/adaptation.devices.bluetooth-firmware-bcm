Name:       bluetooth-firmware-bcm
Summary:    firmware and tools for bluetooth
# Version/Release/Group are based on mobile package
Version:    0.2.29
Release:    1
Group:      Hardware Support/Handset
License:    Apache License, Version 2.0
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

%if %{_repository}=="wearable"
%package msm8974
Summary:    msm8974 firmware and tools for bluetooth
Group:      TO_BE/FILLED

%description msm8974
firmware and tools for bluetooth for redwood msm8974
%endif

%prep
%setup -q -n bluetooth-firmware-bcm-%{version}

%build
export CFLAGS+=" -fpie -fvisibility=hidden"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"

%if %{_repository}=="wearable"
cd wearable
%elseif %{_repository}=="mobile"
cd mobile
%endif

cmake ./ -DCMAKE_INSTALL_PREFIX=%{_prefix} -DPLUGIN_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%if %{_repository}=="wearable"
cd wearable
%elseif %{_repository}=="mobile"
cd mobile
%endif

%make_install

%if %{_repository}=="wearable"
install -D -m 0644 LICENSE.APLv2 %{buildroot}%{_datadir}/license/bluetooth-firmware-bcm-c210
install -D -m 0644 LICENSE.APLv2 %{buildroot}%{_datadir}/license/bluetooth-firmware-bcm-e4412
install -D -m 0644 LICENSE.APLv2 %{buildroot}%{_datadir}/license/bluetooth-firmware-bcm-msm8974
install -D -m 0644 LICENSE.Broadcom %{buildroot}%{_datadir}/license/bluetooth-firmware-bcm-c210
install -D -m 0644 LICENSE.Broadcom %{buildroot}%{_datadir}/license/bluetooth-firmware-bcm-e4412
install -D -m 0644 LICENSE.Broadcom %{buildroot}%{_datadir}/license/bluetooth-firmware-bcm-msm8974
%elseif %{_repository}=="mobile"
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
cat %{_builddir}/%{name}-%{version}/%{_repository}/LICENSE.Broadcom >> %{buildroot}/usr/share/license/%{name}
%endif

%post c210
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%post e4412
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh

%if %{_repository}=="wearable"
%post msm8974
rm -rf %{_prefix}/etc/bluetooth/bt-dev-start.sh
ln -s %{_prefix}/etc/bluetooth/bt-dev-start-msm8974.sh %{_prefix}/etc/bluetooth/bt-dev-start.sh
%endif

%files c210
%manifest %{_repository}/bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%if %{_repository}=="wearable"
%{_prefix}/etc/bluetooth/BCM4330B1_002.001.003.0013.0000_SS-SLP7-B42_NoExtLNA_37_4MHz-TEST-ONLY.hcd
%endif
%{_prefix}/etc/bluetooth/BCM4330B1_002.001.003.0221.0265.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-c210.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
%if %{_repository}=="wearable"
%{_datadir}/license/bluetooth-firmware-bcm-c210
%elseif %{_repository}=="mobile"
/usr/share/license/%{name}
%endif

%files e4412
%manifest %{_repository}/bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%if %{_repository}=="wearable"
%{_prefix}/etc/bluetooth/BCM20710A1_001.002.014.0059.0060.hcd
%{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.1675.1676_B2_ORC.hcd
#%{_prefix}/etc/bluetooth/BCM4334W_001.002.003.0874.0000_Samsung_Rinato_TEST_ONLY.hcd
#%{_prefix}/etc/bluetooth/BCM43342A1_001.002.003.0874.0000_SEMCO_B58_TEST_ONLY.hcd
%{_prefix}/etc/bluetooth/BCM4334W_Rinato_TestOnly.hcd
%{_prefix}/etc/bluetooth/BCM4334W_001.002.003.0997.1027_B58_ePA.hcd
%{_datadir}/license/bluetooth-firmware-bcm-e4412
%elseif %{_repository}=="mobile"
%{_prefix}/etc/bluetooth/BCM4334B0_002.001.013.0079.0081.hcd
/usr/share/license/%{name}
%endif
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-e4412.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh

%if %{_repository}=="wearable"
%files msm8974
%manifest %{_repository}/bluetooth-firmware-bcm.manifest
%defattr(-,root,root,-)
%{_bindir}/bcmtool_4330b1
%{_bindir}/setbd
%{_prefix}/etc/bluetooth/BCM4335B0_002.001.006.0233.0234_ORC_RedWood.hcd
%{_prefix}/etc/bluetooth/BCM4339_003.001.009.0030.0122_ORC_RedWood.hcd
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-end.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-dev-start-msm8974.sh
%attr(755,-,-) %{_prefix}/etc/bluetooth/bt-set-addr.sh
%{_datadir}/license/bluetooth-firmware-bcm-msm8974
%endif
