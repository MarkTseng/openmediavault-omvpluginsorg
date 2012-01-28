<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Marcel Beck <marcel.beck@mbeck.org>
 * @copyright Copyright (c) 2011-2012 Marcel Beck
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

class OmvPluginsOrgRpc extends OMVRpc {

	# Source
	const stableSourceLine = 'deb http://packages.omv-plugins.org ix main';
	const unstableSourceLine = 'deb http://packages.omv-plugins.org donald main';

	# Files
	const stableSourceFile = '/etc/apt/sources.list.d/omv-plugins-org-stable.list';
	const unstableSourceFile = '/etc/apt/sources.list.d/omv-plugins-org-unstable.list';

	var $methodSchemata = array(
            "set" => array('{
                "type":"object",
                "properties":{
                    "use-stable":{"type":"boolean"},
                    "use-unstable":{"type":"boolean"}
                }
            }')
	);

	/**
	 * Get settings directly from os
	 */
	function get() {

		return array(
			'use-stable' => (exec("sudo /bin/sh -c '[ -e ".
				OmvPluginsOrgRpc::stableSourceFile." ] && echo yes'") == 'yes'),
            'use-unstable' => (exec("sudo /bin/sh -c '[ -e ".
                OmvPluginsOrgRpc::unstableSourceFile." ] && echo yes'") == 'yes'),

		);

	}

	/**
	* Set settings
	*/
	function set($data) {

       // Check incoming data
        $this->validateParams(__METHOD__,func_get_args());

        $this->validateSession();

        // Is the user an admin or the same as the one configured for the cert?
        if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
            throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
        }

        $this->commitSession();

		$econf = $this->get();

		// Stable sources
		if(!empty($data['use-stable'])) {
            $cmd = "sudo /bin/sh -c \"echo '". OmvPluginsOrgRpc::stableSourceLine ."' >".
                OmvPluginsOrgRpc::stableSourceFile.'"';
        } else {
            $cmd = "sudo /bin/rm -f ". OmvPluginsOrgRpc::stableSourceFile;
        }
        OMVUtil::exec($cmd, $output, $result);
        if ($result !== 0) {
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
              $cmd, implode("\n", $output));
        }

        // Unstable sources
        if(!empty($data['use-unstable'])) {
            $cmd = "sudo /bin/sh -c \"echo '". OmvPluginsOrgRpc::unstableSourceLine ."' >".
                OmvPluginsOrgRpc::unstableSourceFile.'"';
        } else {
            $cmd = "sudo /bin/rm -f ". OmvPluginsOrgRpc::unstableSourceFile;
        }
        OMVUtil::exec($cmd, $output, $result);
        if ($result !== 0) {
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
              $cmd, implode("\n", $output));
        }

		// apt update?
		if(empty($econf['use-stable']) != empty($data['use-stable']) ||
			empty($econf['use-unstable']) != empty($data['use-unstable'])) {

			$cmd = 'sudo apt-get update';

        	OMVUtil::exec($cmd, $output, $result);
	        if ($result !== 0) {
    	        throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
	              $cmd, implode("\n", $output));
	        }

		}
	}
}

