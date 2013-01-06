<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Marcel Beck <marcel.beck@mbeck.org>
 * @author    Ian Moore <imooreyahoo@gmail.com>
 * @copyright Copyright (c) 2011-2012 Marcel Beck
 * @copyright Copyright (c) 2011 Ian Moore
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/product.inc");

class OmvPluginsOrgRpc extends OMVRpc
{
    private $_repositories = array(
        'ix'             => array(
            'required_version' => '0.2',
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-ix.list',
            'source_line'      => 'deb http://packages.omv-plugins.org ix main',
            'usable'           => false,
        ),
        'omnius'         => array(
            'required_version' => '0.3',
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-omnius.list',
            'source_line'      => 'deb http://packages.omv-plugins.org omnius main',
            'usable'           => false,
        ),
        'fedaykin'       => array(
            'required_version' => '0.4',
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-fedaykin.list',
            'source_line'      => 'deb http://packages.omv-plugins.org fedaykin main',
            'usable'           => false,
        ),
        'fedaykin-armhf' => array(
            'required_version' => '0.4',
            'architecture'     => 'armhf',
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-fedaykin-armhf.list',
            'source_line'      => 'deb http://packages.omv-plugins.org fedaykin-armhf main',
            'usable'           => false,
        ),
    );

    private $_extra_repositories = array(
        'testing' => array(
            'required_version' => '0.4',
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-testing.list',
            'source_line'      => 'deb http://packages.omv-plugins.org testing main',
            'usable'           => false,
        ),
        'donald' => array(
            'required_version' => '0.4',
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-donald.list',
            'source_line'      => 'deb http://packages.omv-plugins.org donald main',
            'usable'           => false,
        ),
        'glass'  => array(
            'required_version' => '0.4',
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-glass.list',
            'source_line'      => 'deb http://packages.omv-plugins.org glass main',
            'usable'           => false,
        ),
    );

    private $_always_repositories = array(
        'squeeze-backports'  => array(
            'source_file'      => '/etc/apt/sources.list.d/omv-plugins-org-squeeze-backports.list',
            'source_line'      => 'deb http://packages.omv-plugins.org squeeze-backports main',
        ),
    );

    var $methodSchemata = array(
        "set" => array(
            '{
					"type":"object",
					"properties":{
							"use-stable":{"type":"boolean"},
							"use-testing":{"type":"boolean"},
							"use-donald":{"type":"boolean"},
							"use-glass":{"type":"boolean"}
					}
			}'
        )
    );

    private $_omv_version;

    private $_omv_version_name;

    public function __construct()
    {
        parent::__construct();
        $prd                     = new OMVProduct();
        $this->_omv_version      = $prd->getVersion();
        $this->_omv_version_name = strtolower($prd->getVersionName());

        foreach ($this->_repositories as $codename => $settings) {
            if (substr($this->_omv_version, 0, 3) == $settings['required_version']) {
                if (isset($settings['architecture'])) {
                    @OMVUtil::exec("dpkg --print-architecture", $output, $result);
                    if ($result !== 0) {
                        continue;
                    }
                    $architecture = $output[0];
                    if ($architecture === $settings['architecture']) {
                        $this->_repositories[$codename]['usable'] = true;
                    }
                } else {
                    $this->_repositories[$codename]['usable'] = true;
                }
            }
        }
        foreach ($this->_extra_repositories as $codename => $settings) {
            if (substr($this->_omv_version, 0, 3) == $settings['required_version']) {
                $this->_extra_repositories[$codename]['usable'] = true;
            }
        }
    }

    /**
     * Get settings directly from os
     */
    function get()
    {
        $result = array(
            'use-stable' => false,
            'use-testing' => false,
            'use-donald' => false,
            'use-glass'  => false,
        );

        if (array_key_exists($this->_omv_version_name, $this->_repositories)) {
            $result['use-stable'] = (exec("sudo /bin/sh -c '[ -e " . $this->_repositories[$this->_omv_version_name]['source_file'] . " ] && echo yes'") == 'yes');
        }

        foreach ($this->_extra_repositories as $codename => $settings) {
            $result['use-' . strtolower($codename)] = (exec("sudo /bin/sh -c '[ -e " . $settings['source_file'] . " ] && echo yes'") == 'yes');
        }
        return $result;
    }

    /**
     * Set settings
     */
    function set($data)
    {
        // Check incoming data
        $this->validateParams(__METHOD__, func_get_args());

        $this->validateSession();

        // Is the user an admin or the same as the one configured for the cert?
        if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
            throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
        }

        $this->commitSession();

        $cmd = "sudo /bin/rm -f /etc/apt/sources.list.d/omv-plugins-org-*.list";
        OMVUtil::exec($cmd, $output, $result);
        if ($result !== 0) {
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
        }

        foreach ($this->_always_repositories as $codename => $settings) {
            $cmd = "sudo /bin/sh -c \"echo '" . $settings['source_line'] . "' >" . $settings['source_file'] . '"';
            OMVUtil::exec($cmd, $output, $result);
            if ($result !== 0) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
            }
        }

        if (!empty($data['use-stable'])) {
            foreach ($this->_repositories as $codename => $settings) {
                if ($settings['usable'] === true) {
                    $cmd = "sudo /bin/sh -c \"echo '" . $settings['source_line'] . "' >" . $settings['source_file'] . '"';
                    OMVUtil::exec($cmd, $output, $result);
                    if ($result !== 0) {
                        throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
                    }
                }
            }
        }

        if (!empty($data['use-testing']) and ($this->_extra_repositories['testing']['usable'] === true)) {
            $cmd = "sudo /bin/sh -c \"echo '" . $this->_extra_repositories['testing']['source_line'] . "' >" . $this->_extra_repositories['testing']['source_file'] . '"';
            OMVUtil::exec($cmd, $output, $result);
            if ($result !== 0) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
            }
        }

        if (!empty($data['use-donald']) and ($this->_extra_repositories['donald']['usable'] === true)) {
            $cmd = "sudo /bin/sh -c \"echo '" . $this->_extra_repositories['donald']['source_line'] . "' >" . $this->_extra_repositories['donald']['source_file'] . '"';
            OMVUtil::exec($cmd, $output, $result);
            if ($result !== 0) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
            }
        }

        if (!empty($data['use-glass']) and ($this->_extra_repositories['glass']['usable'] === true)) {
            $cmd = "sudo /bin/sh -c \"echo '" . $this->_extra_repositories['glass']['source_line'] . "' >" . $this->_extra_repositories['glass']['source_file'] . '"';
            OMVUtil::exec($cmd, $output, $result);
            if ($result !== 0) {
                throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
            }
        }

        $cmd = 'sudo apt-get update';
        OMVUtil::exec($cmd, $output, $result);
        if ($result !== 0) {
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
                                   $cmd, implode("\n", $output));
        }
    }
}

